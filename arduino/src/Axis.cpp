#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::setPower(int16_t target_power) {
  _target_power = target_power;

  int16_t delta = _target_power - _power;
  if (delta > kMax_axis_accel) {
    _power += kMax_axis_accel;
  } else if (delta < kMax_axis_decel) {
    _power += kMax_axis_decel;
  } else {
    _power = _target_power;
  }

  if (_power > 0) {
    _setPower(_power, _invert ^ _reverse);
  } else {
    _setPower(-_power, !(_invert ^ _reverse));
  }
}

void Axis::encoderEdge() {
  uint32_t time = micros();

  _pulse_width = ((uint32_t) (255 - kEncoder_alpha) * _pulse_width + (uint32_t) kEncoder_alpha * (time - _last_edge)) >> 8;
  _last_edge = time;
}
