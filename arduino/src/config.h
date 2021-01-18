#ifndef CONFIG_H
#define CONFIG_H

// min/max of integral state
const int16_t kPID_integral_min = -2000;
const int16_t kPID_integral_max = 2000;

// minimum power to command motor, anything lower will have the motor braked
const uint8_t kMin_motor_threshold = 32;

// max accel/decel
const uint8_t kMax_axis_accel = 16;
const uint8_t kMax_axis_decel = 64;

// alpha for exponential filter used to smooth sensor data
// [0-255], lower for more filtering
const uint8_t kSensor_filter_alpha = 4;

// parser buffer size
// determines max length of command that can be sent
const uint8_t kParser_buf_size = 16;

#endif
