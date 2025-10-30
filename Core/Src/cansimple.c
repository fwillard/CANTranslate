#include "cansimple.h"
#include "canard.h"
#include "cmsis_os.h"
#include <can.h>
#include <string.h>

extern osMessageQueueId_t dronecan_tx_queueHandle;

// CAN SIMPLE CALLBACK
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  // Parse header, dispatch to cansimple_rx_queue
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
    // drop frame and return
    return;
  }

  CANFrame rx_frame;
  rx_frame.id = rx_header.StdId;
  rx_frame.dlc = rx_header.DLC;
  rx_frame.extended =
      (rx_header.IDE == CAN_ID_EXT); // Should never be true for FIFO1
  rx_frame.rtr = rx_header.RTR;
  memcpy(rx_frame.data, rx_data, rx_header.DLC);

  // osMessageQueuePut(cansimple_rx_queueHandle, &rx_frame, 0, 0);
}

void StartCanSimpleTask(void *argument) {
  /* USER CODE BEGIN StartCanSimpleTask */
  (void)argument; // Prevent unused argument warning

  // Initialize CAN Simple library here
  // CanardInstance canard_instance;
  // canardInit(&canard_instance, ...);
  CANFrame frame = {.id = 0x007,
                    .dlc = 4,
                    .extended = false,
                    .rtr = false,
                    .data = {0x08, 0x00, 0x00, 0x00}};

  osStatus_t status = osMessageQueuePut(dronecan_tx_queueHandle, &frame, 0, 0);
  if (status != osOK) {
    // LOG_WARNING("Tx queue full, dropping CAN frame: %d\n", status);
    ; // stop trying if the queue is full
  }

  /* Infinite loop */
  for (;;) {
    // Process CAN Simple messages
    // canardHandleRx(&canard_instance);

    osDelay(10); // Adjust delay as needed
  }
  /* USER CODE END StartCanSimpleTask */
}