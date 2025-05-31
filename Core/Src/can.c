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
  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
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
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
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
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
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
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
  {
    // drop frame and return
    return;
  }
  CanardCANFrame rx_frame;

  rx_frame.id = rx_header.ExtId;
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

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
  osSemaphoreRelease(can_tx_semHandle);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
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

    if (osMessageQueueGet(can_tx_queueHandle, &frame, NULL, osWaitForever) == osOK)
    {
      // Acquire semaphore: wait until at least one mailbox is free
      if (osSemaphoreAcquire(can_tx_semHandle, osWaitForever) == osOK)
      {
        build_tx_header(&frame, &tx_header);

        if (HAL_CAN_AddTxMessage(&hcan, &tx_header, frame.data, &tx_mailbox) != HAL_OK)
        {
          LOG_ERROR("CAN Tx Error\n");

          // Release semaphore to avoid deadlock (mailbox wasn't taken)
          osSemaphoreRelease(can_tx_semHandle);
        }
        else
        {
          LOG_DEBUG("CAN Tx Success: ID=0x%08X\n", tx_header.ExtId);
        }
      }
      else
      {
        LOG_ERROR("CAN Tx Semaphore Error\n");
      }
    }
    else
    {
      LOG_ERROR("CAN Tx Queue Error\n");
    }
  }

  /* USER CODE END StartCANTxTask */
}

/* USER CODE END 1 */
