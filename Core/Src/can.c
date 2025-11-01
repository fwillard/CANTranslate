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
#include "canard.h"
#include "cmsis_os.h"
#include "logging.h"
#include "stm32f2xx_hal_can.h"
#include <stdio.h>
#include <string.h>

extern osMessageQueueId_t dronecan_rx_queueHandle;
extern osMessageQueueId_t cansimple_rx_queueHandle;
extern osMessageQueueId_t dronecan_tx_queueHandle;
extern osSemaphoreId_t dronecan_tx_semaphoreHandle;
extern osSemaphoreId_t cansimple_tx_semaphoreHandle;
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
  CAN_FilterTypeDef can1Filter;

  can1Filter.FilterBank = 0;                     // filter bank number
  can1Filter.FilterMode = CAN_FILTERMODE_IDMASK; // mask mode
  can1Filter.FilterScale = CAN_FILTERSCALE_32BIT;
  can1Filter.FilterIdHigh = 0x0000; // accept all IDs
  can1Filter.FilterIdLow = 0x0000;
  can1Filter.FilterMaskIdHigh = 0x0000; // mask all bits
  can1Filter.FilterMaskIdLow = 0x0000;
  can1Filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // send messages to FIFO0
  can1Filter.FilterActivation = ENABLE;
  can1Filter.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &can1Filter) != HAL_OK) {
    Error_Handler();
  }

  // Start the CAN controller
  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    Error_Handler();
  }

  // Enable receive interrupts
  if (HAL_CAN_ActivateNotification(
          &hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING |
                      CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
    Error_Handler();
  }

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
  CAN_FilterTypeDef can2Filter;

  can2Filter.FilterBank = 14; // filter bank number (0…13 on STM32F1/F4)
  can2Filter.FilterMode = CAN_FILTERMODE_IDMASK; // mask mode
  can2Filter.FilterScale = CAN_FILTERSCALE_32BIT;
  can2Filter.FilterIdHigh = 0x0000; // accept all IDs
  can2Filter.FilterIdLow = 0x0000;
  can2Filter.FilterMaskIdHigh = 0x0000; // mask all bits
  can2Filter.FilterMaskIdLow = 0x0000;
  can2Filter.FilterFIFOAssignment = CAN_FILTER_FIFO1; // send messages to
  can2Filter.FilterActivation = ENABLE;

  if (HAL_CAN_ConfigFilter(&hcan2, &can2Filter) != HAL_OK) {
    Error_Handler();
  }

  // Start the CAN controller

  if (HAL_CAN_Start(&hcan2) != HAL_OK) {
    Error_Handler();
  }

  // Enable receive interrupts
  if (HAL_CAN_ActivateNotification(
          &hcan2, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING |
                      CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
    Error_Handler();
  }

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

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
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

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
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

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
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

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void build_tx_header(const CANFrame *frame, CAN_TxHeaderTypeDef *header) {
  header->IDE = frame->extended ? CAN_ID_EXT : CAN_ID_STD;
  header->RTR = frame->rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA;
  header->DLC = frame->dlc;
  header->TransmitGlobalTime = DISABLE;

  if (frame->extended) {
    header->ExtId = frame->id;
    header->StdId = 0;
  } else {
    header->StdId = frame->id;
    header->ExtId = 0;
  }
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
  if(hcan->Instance == CAN1) {
    osSemaphoreRelease(cansimple_tx_semaphoreHandle);
  }
  else if (hcan->Instance == CAN2) {
    osSemaphoreRelease(dronecan_tx_semaphoreHandle);
  }
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (hcan->Instance == CAN1) {
    osSemaphoreRelease(cansimple_tx_semaphoreHandle);
  }
  else if (hcan->Instance == CAN2) {
    osSemaphoreRelease(dronecan_tx_semaphoreHandle);
  }

}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
  
  if (hcan->Instance == CAN1) {
    osSemaphoreRelease(cansimple_tx_semaphoreHandle);
  }
  else if (hcan->Instance == CAN2) {
    osSemaphoreRelease(dronecan_tx_semaphoreHandle);
  }
}
/* USER CODE END 1 */
