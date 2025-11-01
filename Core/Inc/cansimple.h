#ifndef CANSIMPLE_H
#define CANSIMPLE_H

#include <stdint.h>

#define CANSIMPLE_SET_AXIS_STATE_CMD 0x007

void cansimple_set_axis_state(uint8_t state);

#endif