#include "cansimple.h"
#include "canard.h"
#include "cmsis_os.h"
#include <can.h>

extern osMessageQueueId_t can_tx_queueHandle;

void StartCanSimpleTask(void *argument)
{
    // /* USER CODE BEGIN StartCanSimpleTask */
    // (void)argument; // Prevent unused argument warning

    // // Initialize CAN Simple library here
    // // CanardInstance canard_instance;
    // // canardInit(&canard_instance, ...);
    // CANFrame frame = {
    //     .id = 0x007,
    //     .dlc = 4,
    //     .extended = false,
    //     .rtr = false,
    //     .data = {0x08, 0x00, 0x00, 0x00}};

    // osStatus_t status = osMessageQueuePut(can_tx_queueHandle, &frame, 0, 0);
    // if (status != osOK)
    // {
    //     // LOG_WARNING("Tx queue full, dropping CAN frame: %d\n", status);
    //     ; // stop trying if the queue is full
    // }

    // /* Infinite loop */
    // for (;;)
    // {
    //     // Process CAN Simple messages
    //     // canardHandleRx(&canard_instance);

    //     osDelay(10); // Adjust delay as needed
    // }
    // /* USER CODE END StartCanSimpleTask */
}