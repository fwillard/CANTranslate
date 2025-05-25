#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE 8

#include "canard.h"

typedef struct
{
    CanardCANFrame buffer[QUEUE_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} CanardRxQueue;

int enqueueCanardFrame(CanardRxQueue *queue, CanardCANFrame *frame);
int dequeueCanardFrame(CanardRxQueue *queue, CanardCANFrame *frame);
void canardRxQueueInit(CanardRxQueue *queue);

#endif