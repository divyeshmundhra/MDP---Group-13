#include "motion_profile.h"

void Motion_Profile::init(uint16_t distance) {
  // divide by kMP_max_speed, rounding up
  _min_steps = (distance + (kMP_max_speed - 1)) / kMP_max_speed;
  _phase = PHASE_ACCEL;
  _vel = 0;
  _position = 0;
  _steps = 0;
}

bool Motion_Profile::step(setpoint_t *sp) {
  switch (_phase) {
    case PHASE_IDLE:
      return false;
      break;
    case PHASE_ACCEL:
      sp->vel = pgm_read_byte_near(up_vel_lut + _steps);

      _steps ++;
      if (_steps >= kLen_up_lut) {
        _phase = PHASE_CONSTANT;
      }
      break;
    case PHASE_DECEL:
      sp->vel = pgm_read_byte_near(down_vel_lut + _steps);

      _steps ++;
      if (_steps >= kLen_down_lut) {
        _phase = PHASE_IDLE;
      }
      break;
    case PHASE_CONSTANT:
      sp->vel = kMP_max_speed;

      _steps ++;
      if (_steps >= _min_steps) {
        _phase = PHASE_DECEL;
        _steps = 0;
      }
      break;
  }

  _position += sp->vel;
  sp->pos = _position;
  sp->accel = sp->vel - _vel;
  _vel = sp->vel;

  return true;
}
