#ifndef CANTRANSLATE_H
#define CANTRANSLATE_H

#include "stm32f1xx_hal.h"
#include "canard.h"

#define CANSIMPLE_CMD_GET_VERSION 0x000
#define CANSIMPLE_CMD_HEARTBEAT 0x001
#define CANSIMPLE_CMD_ESTOP 0x002
#define CANSIMPLE_CMD_GET_ERROR 0x003
#define CANSIMPLE_CMD_RXSDO 0x004
#define CANSIMPLE_CMD_TXSDO 0x005
#define CANSIMPLE_CMD_ADDRESS 0x006
#define CANSIMPLE_CMD_SET_AXIS_STATE 0x007
#define CANSIMPLE_CMD_GET_ENCODER_ESTIMATES 0x009
#define CANSIMPLE_CMD_SET_CONTROLLER_MODE 0x00B
#define CANSIMPLE_CMD_SET_INPUT_POS 0x00C
#define CANSIMPLE_CMD_SET_INPUT_VEL 0x00D
#define CANSIMPLE_CMD_SET_INPUT_TORQUE 0x00E
#define CANSIMPLE_CMD_SET_LIMITS 0x00F
#define CANSIMPLE_CMD_SET_TRAJ_VEL_LIMIT 0x011
#define CANSIMPLE_CMD_SET_TRAJ_ACCEL_LIMITS 0x012
#define CANSIMPLE_CMD_SET_TRAJ_INERTIA 0x013
#define CANSIMPLE_CMD_GET_IQ 0x014
#define CANSIMPLE_CMD_GET_TEMPERATURE 0x015
#define CANSIMPLE_CMD_REBOOT 0x016
#define CANSIMPLE_CMD_GET_BUS_VOLTAGE_CURRENT 0x017
#define CANSIMPLE_CMD_CLEAR_ERRORS 0x018
#define CANSIMPLE_CMD_SET_ABSOLUTE_POSITION 0x019
#define CANSIMPLE_CMD_SET_POS_GAIN 0x01A
#define CANSIMPLE_CMD_SET_VEL_GAINS 0x01B
#define CANSIMPLE_CMD_GET_TORQUES 0x01C
#define CANSIMPLE_CMD_GET_POWERS 0x01D
#define CANSIMPLE_CMD_ENTER_DFU_MODE 0x01F

#define CANARD_NODE_ID 97
#define MEMORY_POOL_SIZE 1024

extern CanardInstance canard;
extern uint8_t canard_mem_pool[1024];

extern CAN_HandleTypeDef hcan;

extern volatile uint64_t uptime_millis;

void init_CAN_Translate();
void process_main_loop();
void canardRxOnce();
void canardTx();

void process1HzTasks(uint64_t timestamp_usec);

HAL_StatusTypeDef config_CAN_Filters(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef CAN_Transmit(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t isExtendedId,
                               uint8_t *data, uint8_t size, uint32_t timeout);

void sendNodeStatus();
void handleGetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer);

#endif // CANTRANSLATE_H