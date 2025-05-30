#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdint.h>
extern volatile uint32_t timer_overflows;

uint64_t micros64();

#endif // TIME_UTILS_H