
#include "dronecan.h"
#include "canard.h"
#include "cmsis_os.h"
#include "logging.h"
#include <stdio.h>
#include "utils.h"
#include <dronecan_msgs.h>
#include "can.h"
#include "version.h"
#include <string.h>

#define MAX_TX_FRAMES_PER_LOOP 5 // tune as needed

extern osMessageQueueId_t dronecan_rx_queueHandle;
extern osMessageQueueId_t can_tx_queueHandle;

static CanardInstance canard;
static uint8_t memory_pool[1024];
static struct uavcan_protocol_NodeStatus node_status;

#define NODE_ID 97

// Gets a unique hardware ID for the MCU
// This is specific to the STM32F103, which has a unique ID stored in the
// system memory at address 0x1FFFF7E8. The unique ID is 96 bits long,
// and we will pad it to 128 bits (16 bytes) by adding 4 zero bytes at the end.
void get_unique_id(uint8_t unique_id[16])
{
    // memory location for STM32F103
    const uint32_t *id_base = (const uint32_t *)0x1FFFF7E8;

    // Copy the 96-bit ID
    memcpy(unique_id, id_base, 12);

    // Pad the remaining 4 bytes with zeros
    memset(&unique_id[12], 0, 4);
}

static void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer)
{
    LOG_DEBUG("Handling GetNodeInfo request from node %d\n", transfer->source_node_id);
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
    struct uavcan_protocol_GetNodeInfoResponse pkt;

    memset(&pkt, 0, sizeof(pkt));

    node_status.uptime_sec = micros64() / 1000000ULL;
    pkt.status = node_status;

    pkt.software_version.major = PROJECT_VERSION_MAJOR;
    pkt.software_version.minor = PROJECT_VERSION_MINOR;
    pkt.software_version.optional_field_flags = UAVCAN_PROTOCOL_SOFTWAREVERSION_OPTIONAL_FIELD_FLAG_VCS_COMMIT;
    pkt.software_version.vcs_commit = GIT_HASH;

    pkt.hardware_version.major = 0;
    pkt.hardware_version.minor = 1;
    get_unique_id(pkt.hardware_version.unique_id);

    strncpy((char *)pkt.name.data, "CANTranslate", sizeof(pkt.name.data));
    pkt.name.len = strnlen((char *)pkt.name.data, sizeof(pkt.name.data));

    uint16_t total_size = uavcan_protocol_GetNodeInfoResponse_encode(&pkt, buffer);

    canardRequestOrRespond(ins,
                           transfer->source_node_id,
                           UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
                           UAVCAN_PROTOCOL_GETNODEINFO_ID,
                           &transfer->transfer_id,
                           transfer->priority,
                           CanardResponse,
                           &buffer[0],
                           total_size);
}

static bool shouldAcceptTransfer(const CanardInstance *ins,
                                 uint64_t *out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
    if (transfer_type == CanardTransferTypeRequest)
    {
        // check if we want to handle a specific service request
        switch (data_type_id)
        {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
        {
            *out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE;
            return true;
        }
        }
    }
    // we don't want any other messages
    return false;
}

static void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer)
{
    LOG_DEBUG("Received transfer: ID=%d, Type=%d, Source=%d\n",
              transfer->data_type_id, transfer->transfer_type, transfer->source_node_id);
    // switch on data type ID to pass to the right handler function
    if (transfer->transfer_type == CanardTransferTypeRequest)
    {
        // check if we want to handle a specific service request
        switch (transfer->data_type_id)
        {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
        {
            handle_GetNodeInfo(ins, transfer);
            break;
        }
        }
    }
}

static void send_NodeStatus(void)
{
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];

    node_status.uptime_sec = micros64() / 1000000ULL;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.sub_mode = 0;
    node_status.vendor_specific_status_code = 0;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremeneted on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}

static void process1HzTasks(uint64_t timestamp_usec)
{
    /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    canardCleanupStaleTransfers(&canard, timestamp_usec);

    /*
     Transmit the node status message
     */
    send_NodeStatus();
}

static void processTx(void)
{
    for (int i = 0; i < MAX_TX_FRAMES_PER_LOOP; i++)
    {
        const CanardCANFrame *txf = canardPeekTxQueue(&canard);
        if (txf == NULL)
            break;

        CANFrame frame = {
            .id = txf->id,
            .dlc = txf->data_len,
            .extended = (txf->id & CANARD_CAN_FRAME_EFF) != 0,
            .rtr = (txf->id & CANARD_CAN_FRAME_RTR) != 0,
        };

        memcpy(frame.data, txf->data, frame.dlc);

        osStatus_t status = osMessageQueuePut(can_tx_queueHandle, &frame, 0, 0);
        if (status != osOK)
        {
            LOG_WARNING("Tx queue full, dropping CAN frame: %d\n", status);
            break; // stop trying if the queue is full
        }

        canardPopTxQueue(&canard);
    }
}

void StartDronecanTask(void *argument)
{
    (void)argument; // Prevent unused argument warning

    canardInit(&canard,
               memory_pool,
               sizeof(memory_pool),
               onTransferReceived,
               shouldAcceptTransfer,
               NULL);

    canardSetLocalNodeID(&canard, NODE_ID);

    CanardCANFrame rx_frame;
    osStatus_t status;

    uint64_t next_1hz_service_at = micros64();
    /* Infinite loop */
    for (;;)
    {
        // Wait for a message, but only up to 100 ms so we can do periodic checks
        status = osMessageQueueGet(dronecan_rx_queueHandle, &rx_frame, NULL, 100);
        const uint64_t ts = micros64();

        // Process incoming frame if available
        if (status == osOK)
        {
            LOG_DEBUG("Received CAN frame: ID=0x%03X, DLC=%d\n",
                      rx_frame.id, rx_frame.data_len);
            canardHandleRxFrame(&canard, &rx_frame, ts);
        }
        else if (status != osErrorTimeout)
        {
            LOG_ERROR("DroneCAN RX Queue Error: %d\n", status);
        }

        if (ts >= next_1hz_service_at)
        {
            next_1hz_service_at += 1000000ULL;
            process1HzTasks(ts);
        }

        processTx();
    }
}
