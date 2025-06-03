
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

#define NODE_ID 0

#define PREFERRED_NODE_ID 97

extern osMessageQueueId_t dronecan_rx_queueHandle;
extern osMessageQueueId_t can_tx_queueHandle;

static CanardInstance canard;
static uint8_t memory_pool[1024];
static struct uavcan_protocol_NodeStatus node_status;

/*
  data for dynamic node allocation process
 */
static struct
{
    uint32_t send_next_node_id_allocation_request_at_ms;
    uint32_t node_id_allocation_unique_id_offset;
} DNA;

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

static void request_DNA(void)
{
    const uint32_t now = millis32();

    static uint8_t node_id_allocation_transfer_id = 0;

    DNA.send_next_node_id_allocation_request_at_ms =
        now + UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MIN_REQUEST_PERIOD_MS +
        (random() % UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_FOLLOWUP_DELAY_MS);

    // Structure of the request is documented in the DSDL definition
    // See https://dronecan.github.io/Specification/6._Application_level_functions/#dynamic-node-id-allocation
    uint8_t allocation_request[CANARD_CAN_FRAME_MAX_DATA_LEN - 1];
    allocation_request[0] = (uint8_t)(PREFERRED_NODE_ID << 1U);

    if (DNA.node_id_allocation_unique_id_offset == 0)
    {
        allocation_request[0] |= 1; // First part of unique ID
    }

    uint8_t my_unique_id[16];
    get_unique_id(my_unique_id);

    static const uint8_t MaxLenOfUniqueIDInRequest = 6;
    uint8_t uid_size = (uint8_t)(16 - DNA.node_id_allocation_unique_id_offset);

    if (uid_size > MaxLenOfUniqueIDInRequest)
    {
        uid_size = MaxLenOfUniqueIDInRequest;
    }

    memmove(&allocation_request[1], &my_unique_id[DNA.node_id_allocation_unique_id_offset], uid_size);

    // Broadcasting the request
    const int16_t bcast_res = canardBroadcast(&canard,
                                              UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_SIGNATURE,
                                              UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID,
                                              &node_id_allocation_transfer_id,
                                              CANARD_TRANSFER_PRIORITY_LOW,
                                              &allocation_request[0],
                                              (uint16_t)(uid_size + 1));
    if (bcast_res < 0)
    {
        LOG_DEBUG("Could not broadcast ID allocation req; error %d\n", bcast_res);
    }

    // Preparing for timeout; if response is received, this value will be updated from the callback.
    DNA.node_id_allocation_unique_id_offset = 0;
}

static void handle_DNA_Allocation(CanardInstance *ins, CanardRxTransfer *transfer)
{
    if (canardGetLocalNodeID(&canard) != CANARD_BROADCAST_NODE_ID)
    {
        // already allocated
        return;
    }

    DNA.send_next_node_id_allocation_request_at_ms =
        millis32() + UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MIN_REQUEST_PERIOD_MS +
        (random() % UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_FOLLOWUP_DELAY_MS);

    if (transfer->source_node_id == CANARD_BROADCAST_NODE_ID)
    {
        LOG_DEBUG("Allocation request from another allocatee\n");
        DNA.node_id_allocation_unique_id_offset = 0;
        return;
    }

    // Copying the unique ID from the message
    struct uavcan_protocol_dynamic_node_id_Allocation msg;

    uavcan_protocol_dynamic_node_id_Allocation_decode(transfer, &msg);

    // Obtaining the local unique ID
    uint8_t my_unique_id[sizeof(msg.unique_id.data)];
    get_unique_id(my_unique_id);

    // Matching the received UID against the local one
    if (memcmp(msg.unique_id.data, my_unique_id, msg.unique_id.len) != 0)
    {
        LOG_DEBUG("Mismatching allocation response\n");
        DNA.node_id_allocation_unique_id_offset = 0;
        // No match, return
        return;
    }

    if (msg.unique_id.len < sizeof(msg.unique_id.data))
    {
        // The allocator has confirmed part of unique ID, switching to
        // the next stage and updating the timeout.
        DNA.node_id_allocation_unique_id_offset = msg.unique_id.len;
        DNA.send_next_node_id_allocation_request_at_ms -= UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MIN_REQUEST_PERIOD_MS;

        LOG_DEBUG("Matching allocation response: %d\n", msg.unique_id.len);
    }
    else
    {
        // Allocation complete - copying the allocated node ID from the message
        canardSetLocalNodeID(ins, msg.node_id);
        LOG_DEBUG("Node ID allocated: %d\n", msg.node_id);
    }
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
    if (transfer_type == CanardTransferTypeBroadcast)
    {
        switch (data_type_id)
        {
        case UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID:
        {
            *out_data_type_signature = UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_SIGNATURE;
            return true;
        }
        }
    }
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

    if (transfer->transfer_type == CanardTransferTypeBroadcast)
    {
        switch (transfer->data_type_id)
        {
        case UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID:
        {
            handle_DNA_Allocation(ins, transfer);
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

    if (NODE_ID > 0)
    {
        canardSetLocalNodeID(&canard, NODE_ID);
    }
    else
    {
        LOG_DEBUG("Waiting for DNA node allocation\n");
    }

    CanardCANFrame rx_frame;
    osStatus_t status;

    uint64_t next_1hz_service_at = micros64();
    uint64_t next_50hz_service_at = micros64();
    /* Infinite loop */
    for (;;)
    {
        processTx();

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

        if (canardGetLocalNodeID(&canard) == CANARD_BROADCAST_NODE_ID)
        {
            // we're still waiting for a DNA allocation of our node ID
            if (millis32() > DNA.send_next_node_id_allocation_request_at_ms)
            {
                uint32_t millis = millis32();
                request_DNA();
            }
            continue;
        }

        if (ts >= next_1hz_service_at)
        {
            next_1hz_service_at += 1000000ULL;
            process1HzTasks(ts);
        }
    }
}
