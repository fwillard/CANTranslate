#include "cantranslate.h"
#include "main.h"
#include "queue.h"
#include "timer.h"

#include "version.h"

#include <string.h>
#include <stdio.h>

#include "dronecan_msgs.h"

CanardInstance canard;
uint8_t canard_mem_pool[MEMORY_POOL_SIZE];

CanardRxQueue canardRxQueue;

uint64_t next_1hz_service_at;

static struct uavcan_protocol_NodeStatus node_status;

void print_can_status(CAN_HandleTypeDef *hcan)
{
    uint32_t tsr = hcan->Instance->TSR;

    printf("CAN Status Register: 0x%08lX\n", tsr);
    printf("Mailbox 0: %s\n", (tsr & CAN_TSR_TME0) ? "Empty" : "Full");
    printf("Mailbox 1: %s\n", (tsr & CAN_TSR_TME1) ? "Empty" : "Full");
    printf("Mailbox 2: %s\n", (tsr & CAN_TSR_TME2) ? "Empty" : "Full");

    if (tsr & CAN_TSR_ABRQ0)
        printf("Abort request mailbox 0\n");
    if (tsr & CAN_TSR_ABRQ1)
        printf("Abort request mailbox 1\n");
    if (tsr & CAN_TSR_ABRQ2)
        printf("Abort request mailbox 2\n");

    // Display errors
    uint32_t error_code = HAL_CAN_GetError(hcan);
    printf("CAN Error Code: 0x%08lX\n", error_code);

    // Check ESR register for more diagnostic info
    uint32_t esr = hcan->Instance->ESR;
    printf("Error Status Register: 0x%08lX\n", esr);
    printf("TX Error Counter: %ld\n", (esr & CAN_ESR_TEC) >> CAN_ESR_TEC_Pos);
    printf("RX Error Counter: %ld\n", (esr & CAN_ESR_REC) >> CAN_ESR_REC_Pos);

    if (esr & CAN_ESR_BOFF)
        printf("Bus-Off state\n");
    if (esr & CAN_ESR_EPVF)
        printf("Error Passive state\n");
    if (esr & CAN_ESR_EWGF)
        printf("Error Warning state\n");

    printf("\n");
}

void get_unique_id(uint8_t unique_id[16])
{
    // Correct memory location for STM32F103C6T6
    const uint32_t *id_base = (const uint32_t *)0x1FFFF7E8;

    // Copy the 96-bit ID
    memcpy(unique_id, id_base, 12);

    // Pad the remaining 4 bytes with zeros
    memset(&unique_id[12], 0, 4);
}

HAL_StatusTypeDef config_CAN_Filters(CAN_HandleTypeDef *hcan)
{
    CAN_FilterTypeDef filterConfig;

    filterConfig.FilterBank = 0;
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0004;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0004;
    filterConfig.FilterActivation = ENABLE;
    filterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(hcan, &filterConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    filterConfig.FilterBank = 1;
    filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0000;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0004;

    if (HAL_CAN_ConfigFilter(hcan, &filterConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
    {
        return;
    };
    CanardCANFrame frame;
    frame.id = RxHeader.ExtId | CANARD_CAN_FRAME_EFF;
    frame.data_len = RxHeader.DLC;
    memcpy(frame.data, RxData, RxHeader.DLC);
    frame.iface_id = 0;
    // add frame to the queue
    enqueueCanardFrame(&canardRxQueue, &frame);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);
    // Process CANSimple data here
}

static void onDronecanTransferRecieved(CanardInstance *ins, CanardRxTransfer *transfer)
{
    // switch on data type ID to pass to the right handler function
    if (transfer->transfer_type == CanardTransferTypeRequest)
    {
        // check if we want to handle a specific service request
        switch (transfer->data_type_id)
        {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
        {
            handleGetNodeInfo(ins, transfer);
            break;
        }
        }
    }
}

static bool dronecanShouldAcceptTransfer(const CanardInstance *ins,
                                         uint64_t *out_data_type_signature,
                                         uint16_t data_type_id,
                                         CanardTransferType transfer_type,
                                         uint8_t source_node_id)
{
    // Check if the transfer should be accepted
    if (transfer_type == CanardTransferTypeRequest)
    {
        switch (data_type_id)
        {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
        {
            *out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE;
            return true;
        }
        }
    }
    return false;
}

void process1HzTasks(uint64_t timestamp_usec)
{
    /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    canardCleanupStaleTransfers(&canard, timestamp_usec);

    /*
      Transmit the node status message
    */
    sendNodeStatus();

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the LED on PC13
}

void sendNodeStatus()
{
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];

    node_status.uptime_sec = micros64() / 1000000ULL;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.sub_mode = 0;
    node_status.vendor_specific_status_code = 0;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    static uint8_t transfer_id;

    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}

void handleGetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer)
{
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
    struct uavcan_protocol_GetNodeInfoResponse pkt;

    memset(&pkt, 0, sizeof(pkt));

    node_status.uptime_sec = micros64() / 1000000ULL;
    pkt.status = node_status;

    // fill in your major and minor firmware version
    pkt.software_version.major = PROJECT_VERSION_MAJOR;
    pkt.software_version.minor = PROJECT_VERSION_MINOR;
    pkt.software_version.optional_field_flags = PROJECT_VERSION_PATCH;

    // should fill in hardware version
    pkt.hardware_version.major = 1;
    pkt.hardware_version.minor = 0;

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

void init_CAN_Translate(void)
{
    canardInit(&canard, canard_mem_pool, sizeof(canard_mem_pool), onDronecanTransferRecieved, dronecanShouldAcceptTransfer, NULL);

    canardSetLocalNodeID(&canard, CANARD_NODE_ID);

    if (config_CAN_Filters(&hcan) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);

    HAL_CAN_Start(&hcan);

    next_1hz_service_at = micros64();
}

void canardRxOnce()
{
    CanardCANFrame frame;
    dequeueCanardFrame(&canardRxQueue, &frame);
    canardHandleRxFrame(&canard, &frame, micros64());
}

void canardTx()
{
    for (const CanardCANFrame *txf = NULL; (txf = canardPeekTxQueue(&canard)) != NULL;)
    {
        uint32_t clean_id = txf->id & ~CANARD_CAN_FRAME_EFF;
        HAL_StatusTypeDef status = CAN_Transmit(&hcan, clean_id, (txf->id & CANARD_CAN_FRAME_EFF) != 0, (uint8_t *)txf->data, txf->data_len, 0);
        if (status == HAL_OK) // Success - just drop the frame
        {
            canardPopTxQueue(&canard);
        }
        else // Error - just exit and try again later
        {
            break;
        }
    }
}

void process_main_loop(void)
{
    canardTx();
    canardRxOnce();

    const uint64_t now = micros64();
    if (now >= next_1hz_service_at)
    {
        next_1hz_service_at += 1000000ULL;
        process1HzTasks(now);
    }
}

HAL_StatusTypeDef CAN_Transmit(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t isExtendedId,
                               uint8_t *data, uint8_t size, uint32_t timeout)
{
    print_can_status(hcan); // Print CAN status for debugging

    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    HAL_StatusTypeDef status;

    /* Check input parameters */
    if (hcan == NULL || data == NULL || size > 8)
    {
        return HAL_ERROR;
    }

    TxHeader.StdId = (isExtendedId == 0) ? id : 0;
    TxHeader.ExtId = (isExtendedId != 0) ? id : 0;
    TxHeader.IDE = (isExtendedId != 0) ? CAN_ID_EXT : CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = size; /* Data Length Code (0-8 bytes) */
    TxHeader.TransmitGlobalTime = DISABLE;

    // Check for a free TX mailbox
    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        return HAL_BUSY; // No mailbox available; caller can retry later
    }

    // Try to queue the message
    return HAL_CAN_AddTxMessage(hcan, &TxHeader, data, &TxMailbox);
}
