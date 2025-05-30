
#include "dronecan.h"
#include "canard.h"
#include "cmsis_os.h"
#include "logging.h"
#include "time_utils.h"

extern osMessageQueueId_t dronecan_rx_queueHandle;

// Initialize DroneCAN library here
static CanardInstance canard;
static uint8_t memory_pool[1024];

#define NODE_ID 97

static bool shouldAcceptTransfer(const CanardInstance *ins,
                                 uint64_t *out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
    return true;
}

static void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer)
{
}

void StartDronecanTask(void *argument)
{
    (void)argument; // Prevent unused argument warning

    canardInit(&canard,
               memory_pool,
               sizeof(memory_pool),
               onTransferReceived,
               shouldAcceptTransfer,
               NULL);

    canardSetLocalNodeID(&canard, NODE_ID);

    CanardCANFrame rx_frame;
    osStatus_t status;

    /* Infinite loop */
    for (;;)
    {
        // Wait for a message from the queue (blocks until message available)
        status = osMessageQueueGet(dronecan_rx_queueHandle, &rx_frame, NULL, osWaitForever);
        if (status == osOK)
        {
            canardHandleRxFrame(&canard, &rx_frame, micros64());
        }
        else
        {
            // Handle queue error if needed
            LOG_ERROR("DroneCAN RX Queue Error: %d\n", status);
        }
    }
}