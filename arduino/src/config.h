#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// minimum power to command motor, anything lower will have the motor braked
const uint8_t kMin_motor_threshold = 32;

// max accel/decel
const uint8_t kMax_axis_accel = 32;
const uint8_t kMax_axis_decel = 64;

// alpha for exponential filter used to smooth sensor data
// [0-255], lower for more filtering
const uint8_t kSensor_filter_alpha = 4;

// parser buffer size
// determines max length of command that can be sent
const uint8_t kParser_buf_size = 16;

// controller parameters for offset (minimise error between encoder readings) controller
extern uint16_t kP_offset;
extern uint16_t kI_offset;
extern uint16_t kD_offset;

const int16_t kTL_integral_min = -2000;
const int16_t kTL_integral_max = 2000;

// controller parameters for move-straight controller
extern uint16_t kP_straight;
extern uint16_t kI_straight;

const int16_t kMS_integral_min = -2000;
const int16_t kMS_integral_max = 2000;

const int16_t kMS_max_output = 400;

#endif
