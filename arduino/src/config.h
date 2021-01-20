#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// minimum power to command motor, anything lower will have the motor braked
const uint8_t kMin_motor_threshold = 16;

// alpha for exponential filter used to smooth sensor data
// [0-255], lower for more filtering
const uint8_t kSensor_filter_alpha = 4;

// minimum encoder delta for the robot to be considered moving
const int32_t kEncoder_move_threshold = 10;

// maximum error for move to be completed
const int8_t kMax_encoder_error = 50;

// parser buffer size
// determines max length of command that can be sent
const uint8_t kParser_buf_size = 16;

// controller parameters for offset (minimise error between encoder readings) controller
extern int16_t kP_offset;
extern int16_t kI_offset;
extern int16_t kD_offset;

const int16_t kTL_integral_min = -2000;
const int16_t kTL_integral_max = 2000;

// controller parameters for move-straight controller
extern int16_t kP_straight;
extern int16_t kI_straight;
extern int16_t kD_straight;

const int16_t kMS_integral_min = -400;
const int16_t kMS_integral_max = 400;

const int16_t kMS_max_output = 400;
const int16_t kMS_min_output = -400;

#endif
