/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include "logging.h"
#include "cmsis_os.h"
#include "canard.h"
#include <stdio.h>

extern osMessageQueueId_t dronecan_rx_queueHandle;
extern osMessageQueueId_t cansimple_rx_queueHandle;
extern osMessageQueueId_t can_tx_queueHandle;
extern osSemaphoreId_t can_tx_semHandle;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 3;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Parse header, dispatch to dronecan_rx_queue
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
  {
    // drop frame and return
    return;
  }
  CanardCANFrame rx_frame;

  rx_frame.id = rx_header.ExtId | CANARD_CAN_FRAME_EFF;
  rx_frame.data_len = rx_header.DLC;
  memcpy(rx_frame.data, rx_data, rx_header.DLC);
  rx_frame.iface_id = 0; // Only one CAN interface

  osMessageQueuePut(dronecan_rx_queueHandle, &rx_frame, 0, 0);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Parse header, dispatch to cansimple_rx_queue
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
  {
    // drop frame and return
    return;
  }

  CANFrame rx_frame;
  rx_frame.id = rx_header.StdId;
  rx_frame.dlc = rx_header.DLC;
  rx_frame.extended = (rx_header.IDE == CAN_ID_EXT); // Should never be true for FIFO1
  rx_frame.rtr = rx_header.RTR;
  memcpy(rx_frame.data, rx_data, rx_header.DLC);

  osMessageQueuePut(cansimple_rx_queueHandle, &rx_frame, 0, 0);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
  osSemaphoreRelease(can_tx_semHandle);
}

void build_tx_header(const CANFrame *frame, CAN_TxHeaderTypeDef *header)
{
  header->IDE = frame->extended ? CAN_ID_EXT : CAN_ID_STD;
  header->RTR = frame->rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA;
  header->DLC = frame->dlc;
  header->TransmitGlobalTime = DISABLE;

  if (frame->extended)
  {
    header->ExtId = frame->id;
    header->StdId = 0;
  }
  else
  {
    header->StdId = frame->id;
    header->ExtId = 0;
  }
}

void StartCANTxTask(void *argument)
{
  /* USER CODE BEGIN StartCANTxTask */
  (void)argument; // Prevent unused argument warning

  CANFrame frame;
  CAN_TxHeaderTypeDef tx_header;
  uint32_t tx_mailbox;

  /* Infinite loop */
  for (;;)
  {
    // uint32_t count_before = osMessageQueueGetCount(can_tx_queueHandle);
    // LOG_DEBUG("Queue count before get: %d\n", count_before);
    // if (osMessageQueueGet(can_tx_queueHandle, &frame, NULL, osWaitForever) == osOK)
    // {
    //   // Acquire semaphore: wait until at least one mailbox is free
    //   if (osSemaphoreAcquire(can_tx_semHandle, osWaitForever) == osOK)
    //   {
    //     build_tx_header(&frame, &tx_header);

    //     if (HAL_CAN_AddTxMessage(&hcan, &tx_header, frame.data, &tx_mailbox) != HAL_OK)
    //     {
    //       LOG_ERROR("CAN Tx Error\n");

    //       // Release semaphore to avoid deadlock (mailbox wasn't taken)
    //       osSemaphoreRelease(can_tx_semHandle);
    //     }
    //     else
    //     {
    //       // LOG_DEBUG("CAN Tx Success: ID=0x%08X\n", tx_header.ExtId);
    //       if (osMessageQueueGetCount(can_tx_queueHandle) > 0)
    //       {
    //         // Log the number of frames left in the queue
    //         LOG_DEBUG("CAN Tx Queue Count: %d\n", osMessageQueueGetCount(can_tx_queueHandle));
    //       }
    //     }
    //   }
    //   else
    //   {
    //     LOG_ERROR("CAN Tx Semaphore Error\n");
    //   }
    // }
    // else
    // {
    //   LOG_ERROR("CAN Tx Queue Error\n");
    // }
  }

  /* USER CODE END StartCANTxTask */
}

/* USER CODE END 1 */
