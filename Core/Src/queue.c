#include "queue.h"

#include <string.h>
#include "stm32f1xx_hal.h"

void canardRxQueueInit(CanardRxQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
}

int enqueueCanardFrame(CanardRxQueue *queue, CanardCANFrame *frame)
{
    uint32_t next_head = (queue->head + 1) % QUEUE_SIZE;

    // Disable interrupts to ensure atomic access to the queue
    __disable_irq();

    // Check for overflow (i.e., full queue)
    if (next_head == queue->tail)
    {
        __enable_irq(); // Re-enable interrupts before returning
        return -1;      // Queue is full, can't enqueue
    }

    queue->buffer[queue->head] = *frame; // Copy the frame into the queue
    queue->head = next_head;             // Move head pointer forward

    __enable_irq(); // Re-enable interrupts before returning
    return 0;
}

int dequeueCanardFrame(CanardRxQueue *queue, CanardCANFrame *frame)
{
    __disable_irq();
    if (queue->head == queue->tail)
    {
        __enable_irq(); // Re-enable interrupts before returning
        return -1;      // Queue is empty, nothing to dequeue
    }

    // Read the frame from the tail
    *frame = queue->buffer[queue->tail];
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;

    __enable_irq(); // Re-enable interrupts before returning
    return 0;       // Success
}