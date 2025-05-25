#include "timer.h"
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim2; // This is defined in main.c
static volatile uint32_t overflow_count = 0;

uint64_t micros64(void)
{
    uint32_t high1, high2, low;
    do
    {
        high1 = overflow_count;
        low = __HAL_TIM_GET_COUNTER(&htim2);
        high2 = overflow_count;
    } while (high1 != high2); // avoid race during overflow

    return ((uint64_t)high1 << 16) | low; // only 16 bits of timer
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        overflow_count++;
    }
}