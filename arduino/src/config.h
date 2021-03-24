#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// minimum power to command motor, anything lower will have the motor braked
const uint8_t kMin_motor_threshold = 16;

// alpha for exponential filter used to smooth sensor data
// [0-255], lower for more filtering
const uint8_t kSensor_filter_alpha = 120;
// if the raw ADC values are below this threshold, we treat it as no obstacle found
// without even trying to convert. This handles a potential int16_t overflow
const uint16_t kSensor_min_value = 300;
// maximum delta of sensor distance below which is considered stable
const int8_t kSensor_stable_threshold = 8;

// minimum encoder delta for the robot to be considered moving
const int32_t kEncoder_move_threshold = 10;
// minimum sensor delta to be considered sensor unstable
const int16_t kSensor_delta_threshold = 1;

// maximum error for move-distance to be completed
const int8_t kMax_encoder_error = 5;
// maximum error for move-obstacle to be completed
const int8_t kMax_obstacle_error = 3;
// max difference between both axis for moves to be completed
const int8_t kMax_encoder_diff_error = 5;
// max error for align-equal to be completed
const int8_t kMax_align_error = 2;
// if zero movement seen for this amount of time, end the move
// this should be longer than the report/align delays
const uint16_t kZero_movement_timeout = 400;

// time after a move to wait before reporting sensor values
const int16_t kSensor_report_delay = 100;
// time to delay after an align is complete
const int16_t kAlign_delay = 200;
const int16_t kMax_encoder_correction = 64;
const int16_t kMin_encoder_correction = -64;

// parser buffer size
// determines max length of command that can be sent
const uint8_t kParser_buf_size = 16;

// parameters for aligning the robot to the wall using LEFT_FRONT and LEFT_REAR sensors

// align to wall only if both sensors see something within this distance (mm)
const int16_t kWall_align_max_absolute_threshold = 250;
// align to wall only if the abs difference between both sensors is less than this (mm)
const int16_t kWall_align_max_absolute_difference = 50;
// do offset alignment only if difference between base offset is less than this
// this prevents the robot from trying to correct too large an error
const int16_t kWall_align_max_offset_delta = 20;
// align to wall only if the main controller outputs more than this power
// this prevents the align controller from slowly turning at the end of a move
const int16_t kWall_align_min_power = 100;
// scale difference between sensors
extern int16_t kP_wall_diff_left;
extern int16_t kP_wall_offset_left;

extern int16_t kP_wall_diff_forward;

// wall offsets: target offset to align to when distance to object is < (index * 100)
#define kWall_offset_count 4
const uint8_t kWall_offsets_left[kWall_offset_count] = {
  50,
  60,
  50,
  40
};

// parameters for auto-starting an alignment after a move
// max distance under which an alignment will be started
const int16_t kAuto_align_threshold = 200;
const int16_t kAuto_align_max_diff = 30;
const int16_t kAuto_align_min_diff = 2;

// on axis direction change, add this as a correction to compensate for backlash
const int16_t kBacklash_compensation = 0;

// if FRONT_FRONT_MID sees a value less than this, abort the move immediately
const int16_t kEmergency_brake_threshold = 0;
const int16_t kEmergency_brake_correction = -300;

// max axis acceleration/deceleration
const int16_t kMax_axis_accel = 20;
const int16_t kMax_axis_decel = -400;

// if a forward movement is started while the current sensor value is offset from the ideal position,
// offset the move to try to compensate for the error
#define kForward_align_count 4
const int16_t kForward_align_target[3][kForward_align_count] = {
  // FRONT_FRONT_MID offsets
  {
    0,
    160,
    250,
    330
  },
  // FRONT_FRONT_LEFT offsets
  {
    0,
    160,
    260,
    310
  },
  // FRONT_FRONT_RIGHT offsets
  {
    0,
    145,
    245,
    330
  }
};

// offset move only if error is within this value
const uint8_t kForward_align_max_error = 20;
// always do a offset move if the distance measured by the sensor is less than this value
const int16_t kForward_align_always_align_threshold = 190;

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

const int16_t kMS_integral_min = -100;
const int16_t kMS_integral_max = 100;

const int16_t kMS_max_output = 400;
const int16_t kMS_min_output = -400;

// controller parameters for move-until-obstacle controller
extern int16_t kP_obstacle;
extern int16_t kI_obstacle;
extern int16_t kD_obstacle;

const int16_t kMO_integral_min = -50;
const int16_t kMO_integral_max = 50;

const int16_t kMO_max_output = 100;
const int16_t kMO_min_output = -100;

// controller parameters for align controller
extern int16_t kP_align;
extern int16_t kI_align;
extern int16_t kD_align;

const int16_t kA_integral_min = -50;
const int16_t kA_integral_max = 50;

const int16_t kA_max_output = 100;
const int16_t kA_min_output = -100;

// constants for sensor conversion
// each array corresponds to the constants for one sensor
// same order as sensor_position_t
const double kSensor_constants[6][3] = {
  {-108.795, 1030808.35, 627.38},  // FRONT_FRONT_MID
  {-144.04, 1137481.94, 831.26}, // FRONT_FRONT_RIGHT
  {-104.83, 797470.16, -44.35},   // LEFT_REAR
  {-144.04, 1137481.94, 831.26},  // FRONT_FRONT_LEFT
  {-104.83, 797470.16, -44.35},   // LEFT_FRONT
  {-401.53, 3187629.66, 1600.67}      // RIGHT_FRONT
};

// maximum valid distance reportable by each sensor
const int16_t kSensor_max[6] = {
  650,
  950,
  550,
  950,
  600,
  550
};

#define kSensor_threshold_count 10
// distance thresholds below which there will be an obstacle in that position
/*
  ie
  |0|1|2|3|4|5|6|7|8|9|255|
*/
const int16_t kSensor_thresholds[6][kSensor_threshold_count] = {
  // 2    3    4    5    6    7    8    9    A    B
  { 100, 200, 300, 400, 480, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
  {  80, 180, 300, 400, 500, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
  { 100, 200, 300, 400, 550, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
  {  80, 180, 290, 380, 480, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
  { 100, 200, 300, 400, 500, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
  { 100, 200, 300, 400, 480, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN },
};

const int8_t kSensor_offset[6] = {
  0,
  0,
  0,
  0,
  0,
  0
};

const uint8_t kMovement_buffer_size = 32;

#endif
