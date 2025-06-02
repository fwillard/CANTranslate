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
#include <canard.h>
#include "logging.h"
#include <stdio.h>
#include "time_utils.h"
#include <inttypes.h>
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
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
uint32_t defaultTaskBuffer[ 256 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for dronecan_task */
osThreadId_t dronecan_taskHandle;
uint32_t dronecan_taskBuffer[ 512 ];
osStaticThreadDef_t dronecan_taskControlBlock;
const osThreadAttr_t dronecan_task_attributes = {
  .name = "dronecan_task",
  .cb_mem = &dronecan_taskControlBlock,
  .cb_size = sizeof(dronecan_taskControlBlock),
  .stack_mem = &dronecan_taskBuffer[0],
  .stack_size = sizeof(dronecan_taskBuffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for cansimple_task */
osThreadId_t cansimple_taskHandle;
uint32_t cansimple_taskBuffer[ 256 ];
osStaticThreadDef_t cansimple_taskControlBlock;
const osThreadAttr_t cansimple_task_attributes = {
  .name = "cansimple_task",
  .cb_mem = &cansimple_taskControlBlock,
  .cb_size = sizeof(cansimple_taskControlBlock),
  .stack_mem = &cansimple_taskBuffer[0],
  .stack_size = sizeof(cansimple_taskBuffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for can_tx_task */
osThreadId_t can_tx_taskHandle;
uint32_t can_tx_taskBuffer[ 256 ];
osStaticThreadDef_t can_tx_taskControlBlock;
const osThreadAttr_t can_tx_task_attributes = {
  .name = "can_tx_task",
  .cb_mem = &can_tx_taskControlBlock,
  .cb_size = sizeof(can_tx_taskControlBlock),
  .stack_mem = &can_tx_taskBuffer[0],
  .stack_size = sizeof(can_tx_taskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for logging_task */
osThreadId_t logging_taskHandle;
uint32_t logging_taskBuffer[ 256 ];
osStaticThreadDef_t logging_taskControlBlock;
const osThreadAttr_t logging_task_attributes = {
  .name = "logging_task",
  .cb_mem = &logging_taskControlBlock,
  .cb_size = sizeof(logging_taskControlBlock),
  .stack_mem = &logging_taskBuffer[0],
  .stack_size = sizeof(logging_taskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for dronecan_rx_queue */
osMessageQueueId_t dronecan_rx_queueHandle;
uint8_t dronecan_rx_queueBuffer[ 8 * sizeof( CanardCANFrame ) ];
osStaticMessageQDef_t dronecan_rx_queueControlBlock;
const osMessageQueueAttr_t dronecan_rx_queue_attributes = {
  .name = "dronecan_rx_queue",
  .cb_mem = &dronecan_rx_queueControlBlock,
  .cb_size = sizeof(dronecan_rx_queueControlBlock),
  .mq_mem = &dronecan_rx_queueBuffer,
  .mq_size = sizeof(dronecan_rx_queueBuffer)
};
/* Definitions for log_queue */
osMessageQueueId_t log_queueHandle;
uint8_t log_queueBuffer[ 6 * sizeof( log_message_t ) ];
osStaticMessageQDef_t log_queueControlBlock;
const osMessageQueueAttr_t log_queue_attributes = {
  .name = "log_queue",
  .cb_mem = &log_queueControlBlock,
  .cb_size = sizeof(log_queueControlBlock),
  .mq_mem = &log_queueBuffer,
  .mq_size = sizeof(log_queueBuffer)
};
/* Definitions for cansimple_rx_queue */
osMessageQueueId_t cansimple_rx_queueHandle;
uint8_t cansimple_rx_queueBuffer[ 6 * sizeof( CANFrame ) ];
osStaticMessageQDef_t cansimple_rx_queueControlBlock;
const osMessageQueueAttr_t cansimple_rx_queue_attributes = {
  .name = "cansimple_rx_queue",
  .cb_mem = &cansimple_rx_queueControlBlock,
  .cb_size = sizeof(cansimple_rx_queueControlBlock),
  .mq_mem = &cansimple_rx_queueBuffer,
  .mq_size = sizeof(cansimple_rx_queueBuffer)
};
/* Definitions for can_tx_queue */
osMessageQueueId_t can_tx_queueHandle;
uint8_t can_tx_queueBuffer[ 8 * sizeof( CANFrame ) ];
osStaticMessageQDef_t can_tx_queueControlBlock;
const osMessageQueueAttr_t can_tx_queue_attributes = {
  .name = "can_tx_queue",
  .cb_mem = &can_tx_queueControlBlock,
  .cb_size = sizeof(can_tx_queueControlBlock),
  .mq_mem = &can_tx_queueBuffer,
  .mq_size = sizeof(can_tx_queueBuffer)
};
/* Definitions for micros_mutex */
osMutexId_t micros_mutexHandle;
osStaticMutexDef_t micros_mutexControlBlock;
const osMutexAttr_t micros_mutex_attributes = {
  .name = "micros_mutex",
  .cb_mem = &micros_mutexControlBlock,
  .cb_size = sizeof(micros_mutexControlBlock),
};
/* Definitions for can_tx_sem */
osSemaphoreId_t can_tx_semHandle;
osStaticSemaphoreDef_t can_tx_semControlBlock;
const osSemaphoreAttr_t can_tx_sem_attributes = {
  .name = "can_tx_sem",
  .cb_mem = &can_tx_semControlBlock,
  .cb_size = sizeof(can_tx_semControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void log_task_stack_highwatermarks(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartDronecanTask(void *argument);
extern void StartCanSimpleTask(void *argument);
extern void StartCANTxTask(void *argument);
extern void StartLoggingTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
  called if a stack overflow is detected. */
  /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook
   function is called if a stack overflow is detected. */
  (void)pcTaskName; // Unused parameter
  (void)xTask;      // Unused parameter

  taskDISABLE_INTERRUPTS();
  // You can log the task name here if you have a very safe, minimal way to do so
  // (e.g., set a global variable, toggle an LED)
  // Or just halt.
  for (;;)
    ;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
  /* vApplicationMallocFailedHook() will only be called if
  configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
  function that will get called if a call to pvPortMalloc() fails.
  pvPortMalloc() is called internally by the kernel whenever a task, queue,
  timer or semaphore is created. It is also called by various parts of the
  demo application. If heap_1.c or heap_2.c are used, then the size of the
  heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
  FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
  to query the size of free heap space that remains (although it does not
  provide information on how the remaining heap might be fragmented). */

  LOG_DEBUG("Malloc failed!\n");
  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}
/* USER CODE END 5 */

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

  /* Create the semaphores(s) */
  /* creation of can_tx_sem */
  can_tx_semHandle = osSemaphoreNew(1, 1, &can_tx_sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of dronecan_rx_queue */
  dronecan_rx_queueHandle = osMessageQueueNew (8, sizeof(CanardCANFrame), &dronecan_rx_queue_attributes);

  /* creation of log_queue */
  log_queueHandle = osMessageQueueNew (6, sizeof(log_message_t), &log_queue_attributes);

  /* creation of cansimple_rx_queue */
  cansimple_rx_queueHandle = osMessageQueueNew (6, sizeof(CANFrame), &cansimple_rx_queue_attributes);

  /* creation of can_tx_queue */
  can_tx_queueHandle = osMessageQueueNew (8, sizeof(CANFrame), &can_tx_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dronecan_task */
  dronecan_taskHandle = osThreadNew(StartDronecanTask, NULL, &dronecan_task_attributes);

  /* creation of cansimple_task */
  cansimple_taskHandle = osThreadNew(StartCanSimpleTask, NULL, &cansimple_task_attributes);

  /* creation of can_tx_task */
  can_tx_taskHandle = osThreadNew(StartCANTxTask, NULL, &can_tx_task_attributes);

  /* creation of logging_task */
  logging_taskHandle = osThreadNew(StartLoggingTask, NULL, &logging_task_attributes);

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
  /* Infinite loop */
  for (;;)
  {
    log_task_stack_highwatermarks();
    osDelay(60000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void log_task_stack_highwatermarks(void)
{
  LOG_DEBUG("Default task stack high water mark: %u\n", uxTaskGetStackHighWaterMark(defaultTaskHandle));
  LOG_DEBUG("Dronecan task stack high water mark: %u\n", uxTaskGetStackHighWaterMark(dronecan_taskHandle));
  LOG_DEBUG("Cansimple task stack high water mark: %u\n", uxTaskGetStackHighWaterMark(cansimple_taskHandle));
  LOG_DEBUG("Can Tx task stack high water mark: %u\n", uxTaskGetStackHighWaterMark(can_tx_taskHandle));
  LOG_DEBUG("Logging task stack high water mark: %u\n", uxTaskGetStackHighWaterMark(logging_taskHandle));
  LOG_DEBUG("\n");
}

/* USER CODE END Application */

