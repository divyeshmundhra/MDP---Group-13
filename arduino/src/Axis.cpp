#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::encoderEdge() {
  _encoder_count ++;
}

void Axis::updateSpeed() {
  if (_target_speed > (_cur_speed + kMax_axis_accel)) {
    _cur_speed += kMax_axis_accel;
  } else if (_target_speed < (_cur_speed - kMax_axis_decel)) {
    _cur_speed -= kMax_axis_decel;
  } else {
    _cur_speed = _target_speed;
  }

  if (_cur_speed > 0) {
    _setSpeed(_cur_speed, !_reverse);
  } else {
    _setSpeed(-_cur_speed, _reverse);
  }
}
