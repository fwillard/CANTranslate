#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "can_interface.h"
#include <string.h>

extern QueueHandle_t dronecan_rx_queue;
extern QueueHandle_t cansimple_rx_queue;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
    {
        // Handle error
        return;
    }

    // Only extended ID frames expected here
    if (rx_header.IDE != CAN_ID_EXT)
    {
        // Unexpected frame type in FIFO0
        return;
    }

    CANRxFrame frame;
    frame.id = rx_header.ExtId;
    frame.is_extended = 1; // Extended ID
    frame.dlc = rx_header.DLC;
    memcpy(frame.data, rx_data, rx_header.DLC);
    frame.timestamp_us = 0;

    // xQueueSendFromISR(dronecan_rx_queue, &frame, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}