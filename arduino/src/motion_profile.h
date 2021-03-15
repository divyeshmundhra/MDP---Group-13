#ifndef MOTION_PROFILE_H
#define MOTION_PROFILE_H

#include <Arduino.h>
#include <stdint.h>

// built with reference to https://www.chiefdelphi.com/t/motion-profiling/115133/18

typedef struct {
  int8_t vel;
  int32_t pos;
  int8_t accel;
} setpoint_t;

// in encoder ticks/s
const uint8_t kMP_max_speed = 28;
const uint8_t kLen_up_lut = 18;
const PROGMEM int8_t up_vel_lut[kLen_up_lut] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 19, 21, 23, 24, 25, 26, 27, 27, 27};
const uint8_t kLen_down_lut = 19;
const PROGMEM int8_t down_vel_lut[kLen_down_lut] = {26, 25, 23, 21, 19, 17, 15, 13, 11, 8, 6, 4, 3, 2, 1, 0, 0, 0, 0};
class Motion_Profile {
  public:
    void init(uint16_t distance);
    bool step(setpoint_t *sp);
  private:
    typedef enum {
      PHASE_IDLE,
      PHASE_ACCEL,
      PHASE_CONSTANT,
      PHASE_DECEL
    } phase_t;

    phase_t _phase;

    // num of steps to cover distance if always at max speed
    // named N in reference
    int16_t _min_steps;
    // number of steps elapsed
    int16_t _steps;
    // last position setpoint
    int32_t _position;
    // last velocity setpoint
    int8_t _vel;
};

#endif
