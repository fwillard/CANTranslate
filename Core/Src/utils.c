#include "utils.h"
#include "cmsis_os.h"
#include "stm32f2xx_hal.h"

volatile uint32_t timer_overflows = 0;
extern osMutexId_t micros_mutexHandle;
extern TIM_HandleTypeDef htim2;

uint32_t state = 0;

uint64_t micros64() {
  osMutexAcquire(micros_mutexHandle, osWaitForever);

  uint16_t timer_count = __HAL_TIM_GET_COUNTER(&htim2);
  uint64_t timestamp = ((uint64_t)timer_overflows << 16) | timer_count;

  osMutexRelease(micros_mutexHandle);
  return timestamp;
}

uint32_t millis32() { return micros64() / 1000; }

uint32_t random() {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

void random_seed(uint32_t seed) { state = seed; }

float rand_float_range(float min, float max) {
  return min + ((float)(random() % 10000) / 10000.0f) * (max - min);
}
