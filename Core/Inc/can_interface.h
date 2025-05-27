#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <stdint.h>

typedef struct
{
    uint32_t id;
    uint8_t is_extended;
    uint8_t dlc;
    uint8_t data[8];
    uint64_t timestamp_us;
} CANRxFrame;

#endif // CAN_INTERFACE_H