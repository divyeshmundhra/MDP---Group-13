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

uint16_t Axis::getSpeed() {
  // rpm = 60 / ( pulse_width * 1124.5/1000000 )
  // 53357 = 60 / ( 1124.5/1000000 )

  if (_pulse_width > kEncoder_timeout) {
    return 0;
  }

  return 53357 / _pulse_width;
}

void Axis::controllerSpeed() {
  // expected to be called from an ISR, does not handle atomic reads of variables
  int16_t integral = 0;

  int16_t error = _target_speed - getSpeed();
  integral = constrain(integral + error, kSpeed_integral_min, kSpeed_integral_max);
  int16_t power = ((int32_t) kP_speed * error + (int32_t) kI_speed * integral) >> 8;

  if (power < 0) {
    power = 0;
  }

  setPower(power);
}
