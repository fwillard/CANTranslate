/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "canard.h"
#include "logging.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for dronecanTask */
osThreadId_t dronecanTaskHandle;
const osThreadAttr_t dronecanTask_attributes = {
  .name = "dronecanTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for cansimpleTask */
osThreadId_t cansimpleTaskHandle;
const osThreadAttr_t cansimpleTask_attributes = {
  .name = "cansimpleTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for dronecanTxTask */
osThreadId_t dronecanTxTaskHandle;
const osThreadAttr_t dronecanTxTask_attributes = {
  .name = "dronecanTxTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for dronecan_tx_queue */
osMessageQueueId_t dronecan_tx_queueHandle;
const osMessageQueueAttr_t dronecan_tx_queue_attributes = {
  .name = "dronecan_tx_queue"
};
/* Definitions for dronecan_rx_queue */
osMessageQueueId_t dronecan_rx_queueHandle;
const osMessageQueueAttr_t dronecan_rx_queue_attributes = {
  .name = "dronecan_rx_queue"
};
/* Definitions for cansimple_rx_queue */
osMessageQueueId_t cansimple_rx_queueHandle;
const osMessageQueueAttr_t cansimple_rx_queue_attributes = {
  .name = "cansimple_rx_queue"
};
/* Definitions for log_queue */
osMessageQueueId_t log_queueHandle;
const osMessageQueueAttr_t log_queue_attributes = {
  .name = "log_queue"
};
/* Definitions for micros_mutex */
osMutexId_t micros_mutexHandle;
const osMutexAttr_t micros_mutex_attributes = {
  .name = "micros_mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartDronecanTask(void *argument);
extern void StartCanSimpleTask(void *argument);
extern void StartDronecanTxTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
  called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of micros_mutex */
  micros_mutexHandle = osMutexNew(&micros_mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of dronecan_tx_queue */
  dronecan_tx_queueHandle = osMessageQueueNew (16, sizeof(CANFrame), &dronecan_tx_queue_attributes);

  /* creation of dronecan_rx_queue */
  dronecan_rx_queueHandle = osMessageQueueNew (16, sizeof(CanardCANFrame), &dronecan_rx_queue_attributes);

  /* creation of cansimple_rx_queue */
  cansimple_rx_queueHandle = osMessageQueueNew (16, sizeof(CANFrame), &cansimple_rx_queue_attributes);

  /* creation of log_queue */
  log_queueHandle = osMessageQueueNew (16, sizeof(log_message_t), &log_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dronecanTask */
  dronecanTaskHandle = osThreadNew(StartDronecanTask, NULL, &dronecanTask_attributes);

  /* creation of cansimpleTask */
  cansimpleTaskHandle = osThreadNew(StartCanSimpleTask, NULL, &cansimpleTask_attributes);

  /* creation of dronecanTxTask */
  dronecanTxTaskHandle = osThreadNew(StartDronecanTxTask, NULL, &dronecanTxTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  for (;;) {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

