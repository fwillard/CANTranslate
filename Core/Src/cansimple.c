#include "cansimple.h"
#include "canard.h"
#include "cmsis_os.h"
#include "logging.h"
#include <can.h>
#include <string.h>

extern osMessageQueueId_t cansimple_tx_queueHandle;
extern osSemaphoreId_t cansimple_tx_semaphoreHandle;

void cansimple_set_axis_state(uint8_t state) {
  // Placeholder function to set the arm state
  // Implement the actual logic as needed
  CANFrame frame;
  frame.id = CANSIMPLE_SET_AXIS_STATE_CMD;
  frame.dlc = 1;
  frame.extended = 0;
  frame.rtr = 0;
  if (state == 255) {
    frame.data[0] = 0x8; // closed loop control
  } else {
    frame.data[0] = 0x1; // idle
  }

  osStatus_t status = osMessageQueuePut(cansimple_tx_queueHandle, &frame, 0, 0);
  if (status != osOK) {
    LOG_WARNING("Tx queue full, dropping CAN frame: %d\n", status);
    return; // stop trying if the queue is full
  }
}

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
  rx_frame.extended = (rx_header.IDE == CAN_ID_EXT);
  rx_frame.rtr = rx_header.RTR;
  memcpy(rx_frame.data, rx_data, rx_header.DLC);

  // osMessageQueuePut(cansimple_rx_queueHandle, &rx_frame, 0, 0);
}

void StartCanSimpleTask(void *argument) {
  /* USER CODE BEGIN StartCanSimpleTask */
  (void)argument; // Prevent unused argument warning

  /* Infinite loop */
  for (;;) {
    // Process CAN Simple messages
    // canardHandleRx(&canard_instance);

    osDelay(1000); // Adjust delay as needed
  }
  /* USER CODE END StartCanSimpleTask */
}

void StartCanSimpleTxTask(void *argument) {
  /* USER CODE BEGIN StartCANTxTask */
  (void)argument; // Prevent unused argument warning

  CANFrame frame;
  CAN_TxHeaderTypeDef tx_header;
  uint32_t tx_mailbox;

  /* Infinite loop */
  for (;;) {
    LOG_INFO("TEST");
    if (osMessageQueueGet(cansimple_tx_queueHandle, &frame, NULL,
                          osWaitForever) == osOK) {
      build_tx_header(&frame, &tx_header);

      osSemaphoreAcquire(cansimple_tx_semaphoreHandle, 10);
      HAL_StatusTypeDef status =
          HAL_CAN_AddTxMessage(&hcan1, &tx_header, frame.data, &tx_mailbox);

      if (status != HAL_OK) {
        LOG_ERROR("CAN Tx Error: %d\n", status);
        uint32_t tsr = READ_REG(hcan1.Instance->TSR);
        bool mailbox0_free = (tsr & CAN_TSR_TME0) != 0;
        bool mailbox1_free = (tsr & CAN_TSR_TME1) != 0;
        bool mailbox2_free = (tsr & CAN_TSR_TME2) != 0;

        LOG_ERROR("Mailbox 0: %s\n", mailbox0_free ? "FREE" : "FULL");
        LOG_ERROR("Mailbox 1: %s\n", mailbox1_free ? "FREE" : "FULL");
        LOG_ERROR("Mailbox 2: %s\n", mailbox2_free ? "FREE" : "FULL");
      }
    }
    /* USER CODE END StartCANTxTask */
  }
}