#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// minimum power to command motor, anything lower will have the motor braked
const uint8_t kMin_motor_threshold = 16;

// alpha for exponential filter used to smooth sensor data
// [0-255], lower for more filtering
const uint8_t kSensor_filter_alpha = 64;

// max axis acceleration/deceleration
const int16_t kMax_axis_accel = 64;
const int16_t kMax_axis_decel = -128;

// minimum encoder delta for the robot to be considered moving
const int32_t kEncoder_move_threshold = 10;
// minimum sensor delta to be considered sensor unstable
const int16_t kSensor_delta_threshold = 1;

// maximum error for move-distance to be completed
const int8_t kMax_encoder_error = 5;
// maximum error for move-obstacle to be completed
const int8_t kMax_obstacle_error = 5;
// max difference between both axis for moves to be completed
const int8_t kMax_encoder_diff_error = 5;
// max error for wall align to be completed (mm)
const int8_t kMax_align_error = 2;

// parser buffer size
// determines max length of command that can be sent
const uint8_t kParser_buf_size = 16;

// controller parameters for offset (minimise error between encoder readings) controller
extern int16_t kP_offset;
extern int16_t kI_offset;
extern int16_t kD_offset;

const int16_t kTL_integral_min = -400;
const int16_t kTL_integral_max = 400;

// controller parameters for move-straight controller
extern int16_t kP_straight;
extern int16_t kI_straight;
extern int16_t kD_straight;

const int16_t kMS_integral_min = -400;
const int16_t kMS_integral_max = 400;

const int16_t kMS_max_output = 400;
const int16_t kMS_min_output = -400;

// controller parameters for move-until-obstacle controller
extern int16_t kP_obstacle;
extern int16_t kI_obstacle;
extern int16_t kD_obstacle;

const int16_t kMO_integral_min = -400;
const int16_t kMO_integral_max = 400;

const int16_t kMO_max_output = 400;
const int16_t kMO_min_output = -400;

// controller parameters for wall-align controller
extern int16_t kP_align;
extern int16_t kI_align;
extern int16_t kD_align;

const int16_t kWA_integral_min = -80;
const int16_t kWA_integral_max = 80;

// constants for sensor conversion
// each array corresponds to the constants for one sensor
// same order as sensor_position_t
const double kSensor_constants[6][3] = {
  {-94.94, 748500, -19.02},  // FRONT_FRONT_MID
  {-141.4, 1559000, -114.8}, // FRONT_FRONT_RIGHT
  {-123.3, 707900, 11.64},   // LEFT_REAR
  {-170.8, 1789000, 114.5},  // FRONT_FRONT_LEFT
  {-123.3, 707900, 11.64},   // LEFT_FRONT
  {-131, 821700, 137.6}      // RIGHT_FRONT
};

#endif
