#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::setSpeed(int16_t target_speed) {
  _target_speed = target_speed;

  int16_t delta = _target_speed - _speed;
  if (delta > kMax_axis_accel) {
    _speed += kMax_axis_accel;
  } else if (delta < kMax_axis_decel) {
    _speed += kMax_axis_decel;
  } else {
    _speed = _target_speed;
  }

  if (_speed > 0) {
    _setSpeed(_speed, _invert ^ _reverse);
  } else {
    _setSpeed(-_speed, !(_invert ^ _reverse));
  }
}
