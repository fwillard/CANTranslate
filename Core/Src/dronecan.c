
#include "dronecan.h"
#include "canard.h"
#include "cmsis_os.h"
#include "logging.h"

extern osMessageQueueId_t dronecan_rx_queueHandle;

void ProcessDroneCANFrame(CanardCANFrame *frame)
{
    // Example processing - you'll customize this based on your DroneCAN protocol needs
    LOG_DEBUG("Received CAN Frame: ID=0x%08X, Len=%d\n",
              frame->id, frame->data_len);

    LOG_DEBUG("\n");
}

void StartDronecanTask(void *argument)
{
    (void)argument; // Prevent unused argument warning

    // Initialize DroneCAN library here
    // CanardInstance canard_instance;
    // canardInit(&canard_instance, ...);

    CanardCANFrame rx_frame;
    osStatus_t status;

    /* Infinite loop */
    for (;;)
    {
        // Wait for a message from the queue (blocks until message available)
        status = osMessageQueueGet(dronecan_rx_queueHandle, &rx_frame, NULL, osWaitForever);
        if (status == osOK)
        {
            // Process the received CAN frame
            ProcessDroneCANFrame(&rx_frame);
        }
        else
        {
            // Handle queue error if needed
            LOG_ERROR("DroneCAN RX Queue Error: %d\n", status);
        }
    }
}