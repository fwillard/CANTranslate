#ifndef LOGGING_H
#define LOGGING_H

#include "cmsis_os.h"
#include <stdarg.h>
#include <stdint.h>
#include <sys/_intsup.h>

#define LOG_MAX_MESSAGE_SIZE 256
#define LOG_MAX_TASK_NAME_SIZE 16

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_MAGENTA "\033[35m"

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_NONE,
} log_level_t;

typedef struct {
  log_level_t level;
  uint32_t timestamp;
  char task_name[LOG_MAX_TASK_NAME_SIZE];
  char message[LOG_MAX_MESSAGE_SIZE];
} log_message_t;

// Public API
void log_message(log_level_t level, const char *format, ...);

#define LOG_DEBUG(fmt, ...) log_message(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_message(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log_message(LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_message(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif // LOGGING_H