#ifndef CONFIG_H
#define CONFIG_H

const int32_t kP_left = 200;
const int32_t kI_left = 0;
const int32_t kD_left = 0;

const int32_t kP_right = 200;
const int32_t kI_right = 0;
const int32_t kD_right = 0;

// min/max of integral state
const int16_t kPID_integral_min = -2000;
const int16_t kPID_integral_max = 2000;

// alpha for encoder filtering
// smaller for more aggressive filtering
const uint8_t kEncoder_alpha = 25;

// timeout (us) for time since encoder pulse received
// after which a motor is treated as stationary
const uint16_t kEncoder_timeout = 10000;

#endif
