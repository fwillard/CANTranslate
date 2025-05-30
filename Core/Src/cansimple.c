#include "cansimple.h"
#include "canard.h"
#include "cmsis_os.h"

void StartCanSimpleTask(void *argument)
{
    /* USER CODE BEGIN StartCanSimpleTask */
    (void)argument; // Prevent unused argument warning

    // Initialize CAN Simple library here
    // CanardInstance canard_instance;
    // canardInit(&canard_instance, ...);

    /* Infinite loop */
    for (;;)
    {
        // Process CAN Simple messages
        // canardHandleRx(&canard_instance);

        osDelay(10); // Adjust delay as needed
    }
    /* USER CODE END StartCanSimpleTask */
}