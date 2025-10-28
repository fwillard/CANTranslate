#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
extern volatile uint32_t timer_overflows;

uint64_t micros64(void);
uint32_t millis32(void);
uint32_t random(void);
void random_seed(uint32_t seed);
float rand_float_range(float min, float max);

#endif // UTILS_H