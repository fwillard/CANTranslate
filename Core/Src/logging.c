#include "logging.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern osMessageQueueId_t log_queueHandle;
extern osThreadId_t logging_taskHandle;

// Logging task implementation
void StartLoggingTask(void *argument) {
  (void)argument;
  log_message_t log_msg;
  osStatus_t status;

  const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR"};
  const char *level_colors[] = {COLOR_CYAN, COLOR_GREEN, COLOR_YELLOW,
                                COLOR_RED};

  printf("Logging task started\n");

  for (;;) {
    // Check if queue handle is valid
    if (log_queueHandle == NULL) {
      printf("ERROR: log_queueHandle is NULL!\n");
      osDelay(1000);
      continue;
    }

    status = osMessageQueueGet(log_queueHandle, &log_msg, NULL, osWaitForever);

    if (status == osOK) {
      uint32_t ms = log_msg.timestamp % 1000;
      uint32_t sec = (log_msg.timestamp / 1000) % 60;
      uint32_t min = (log_msg.timestamp / (1000 * 60)) % 60;
      uint32_t hr = (log_msg.timestamp / (1000 * 60 * 60)) % 24;

      // Print with timestamp and level
      printf("%s[%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32
             "] [%s] [%s] %s%s\n",
             level_colors[log_msg.level], hr, min, sec, ms, log_msg.task_name,
             level_strings[log_msg.level], log_msg.message, COLOR_RESET);
    } else {
      printf("ERROR: osMessageQueueGet failed with status: %d\n", status);
      osDelay(100); // Small delay to prevent spam
    }
  }
}

// Thread-safe logging function
void log_message(log_level_t level, const char *format, ...) {
  if (log_queueHandle == NULL) {
    // Fallback to direct printf if queue not available
    printf("LOG QUEUE NOT READY: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return;
  }

  log_message_t log_msg;
  va_list args;

  const char *task_name = osThreadGetName(osThreadGetId());

  if (task_name == NULL) {
    strncpy(log_msg.task_name, "Unknown", LOG_MAX_TASK_NAME_SIZE);
  } else {
    strncpy(log_msg.task_name, task_name, LOG_MAX_TASK_NAME_SIZE);
  }
  log_msg.task_name[LOG_MAX_TASK_NAME_SIZE - 1] = '\0';

  // Format the message
  va_start(args, format);
  vsnprintf(log_msg.message, LOG_MAX_MESSAGE_SIZE, format, args);
  va_end(args);

  // Fill in metadata
  log_msg.level = level;
  log_msg.timestamp = osKernelGetTickCount() * portTICK_PERIOD_MS;

  // Send to logging task (non-blocking to avoid deadlocks)
  osStatus_t status = osMessageQueuePut(log_queueHandle, &log_msg, 0, 0);

  if (status != osOK) {
    // Queue full or other error - could implement overflow handling here
    // For now, just drop the message to avoid blocking
  }
}