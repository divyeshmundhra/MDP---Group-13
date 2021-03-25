#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::setPower(int16_t target_power, bool cap_accel) {
  _target_power = target_power;

  if (!cap_accel) {
    _power = _target_power;
  } else {
    int16_t delta = _target_power - _power;
    if (delta > kMax_axis_accel) {
      _power += kMax_axis_accel;
    } else if (delta < kMax_axis_decel) {
      _power += kMax_axis_decel;
    } else {
      _power = _target_power;
    }
  }

  if (_power > 0) {
    _setPower(_power, _invert ^ _reverse);
  } else {
    _setPower(-_power, !(_invert ^ _reverse));
  }
}

void Axis::setReverse(bool reverse) {
  static char pReverse = 2;

  if (pReverse != 2) {
    if (reverse != pReverse) {
      incrementEncoder(kBacklash_compensation);
    }
  }

  pReverse = reverse;

  _reverse = reverse;
}

void Axis::resetEncoderForNextMove(int32_t error) {
  if (_invert ^ _reverse) {
    _encoder_count = error;
  } else {
    _encoder_count = -error;
  }
  _encoder_count = 0;
  _encoder_correction = 0;
}
