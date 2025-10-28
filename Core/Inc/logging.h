#ifndef LOGGING_H
#define LOGGING_H

#include "cmsis_os.h"
#include <stdarg.h>
#include <stdint.h>

#define LOG_MAX_MESSAGE_SIZE 256

typedef enum
{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
} log_level_t;

typedef struct
{
    log_level_t level;
    uint32_t timestamp;
    char message[LOG_MAX_MESSAGE_SIZE];
} log_message_t;

// Public API
void log_message(log_level_t level, const char *format, ...);

#define LOG_DEBUG(fmt, ...) log_message(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_message(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log_message(LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_message(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif // LOGGING_H