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

extern osMessageQueueId_t dronecan_rx_queueHandle;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  // Initialize the CAN filters. Send extended frames to RX FIFO 0 and standard frames to RX FIFO 1.
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

  if (HAL_CAN_ConfigFilter(&hcan, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  filterConfig.FilterBank = 1;
  filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
  filterConfig.FilterIdHigh = 0x0000;
  filterConfig.FilterIdLow = 0x0000;
  filterConfig.FilterMaskIdHigh = 0x0000;
  filterConfig.FilterMaskIdLow = 0x0004;

  if (HAL_CAN_ConfigFilter(&hcan, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  // Start the CAN controller
  if (HAL_CAN_Start(&hcan) != HAL_OK)
  {
    Error_Handler();
  }

  // Enable receive interrupts
  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END CAN_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (canHandle->Instance == CAN1)
  {
    /* USER CODE BEGIN CAN1_MspInit 0 */

    /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    /* USER CODE BEGIN CAN1_MspInit 1 */

    /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{

  if (canHandle->Instance == CAN1)
  {
    /* USER CODE BEGIN CAN1_MspDeInit 0 */

    /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    /* USER CODE BEGIN CAN1_MspDeInit 1 */

    /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Parse header, dispatch to dronecan_rx_queue
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
  {
    // drop frame and return
    return;
  }
  CanardCANFrame rx_frame;

  rx_frame.id = rxHeader.ExtId;
  rx_frame.data_len = rxHeader.DLC;
  memcpy(rx_frame.data, rxData, rxHeader.DLC);
  rx_frame.iface_id = 0; // Only one CAN interface

  osMessageQueuePut(dronecan_rx_queueHandle, &rx_frame, 0, 0);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Dispatch to cansimple_rx_queue
}

void StartCANTxTask(void *argument)
{
  /* USER CODE BEGIN StartCANTxTask */
  (void)argument; // Prevent unused argument warning

  osDelay(100);
  uint8_t count = 0;
  /* Infinite loop */
  for (;;)
  {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data = count++;
    uint32_t tx_mailbox;

    tx_header.ExtId = 0x12345678;           // Example ID
    tx_header.IDE = CAN_ID_EXT;             // Extended ID
    tx_header.RTR = CAN_RTR_DATA;           // Data frame
    tx_header.DLC = 1;                      // Data length
    tx_header.TransmitGlobalTime = DISABLE; // No global time                  // Incrementing data for demonstration

    if (HAL_CAN_AddTxMessage(&hcan, &tx_header, &tx_data, &tx_mailbox) != HAL_OK)
    {
      // Handle error
      LOG_ERROR("CAN Tx Error\n");
    }
    else
    {
      LOG_DEBUG("CAN Tx Success: ID=0x%08X, Data=%02X\n", tx_header.ExtId, tx_data);
    }

    osDelay(1000); // Delay for 1 second before next transmission
  }
  /* USER CODE END StartCANTxTask */
}

/* USER CODE END 1 */
