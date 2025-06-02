#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
extern volatile uint32_t timer_overflows;

uint64_t micros64();
uint32_t millis32();

#endif // UTILS_H