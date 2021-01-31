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

void Axis::encoderEdge(int8_t delta) {
  uint32_t time = micros();

  _pulse_width = ((uint32_t) (255 - kEncoder_alpha) * _pulse_width + (uint32_t) kEncoder_alpha * (time - _last_edge)) >> 8;
  _last_edge = time;

  _last_encoder_dir = delta;
}

int16_t Axis::getVelocity() {
  // rpm = 60 / ( pulse_width * 1124.5/1000000 )
  // 53357 = 60 / ( 1124.5/1000000 )

  if ((micros() - _last_edge) > kEncoder_timeout) {
    return 0;
  }

  if (_last_encoder_dir >= 0 && !_reverse) {
    return 53357 / _pulse_width;
  } else {
    return -53357 / _pulse_width;
  }
}

void Axis::controllerVelocity() {
  // expected to be called from an ISR, does not handle atomic reads of variables
  static int16_t integral = 0;

  int16_t error = _target_velocity - getVelocity();
  integral = constrain(integral + error, kSpeed_integral_min, kSpeed_integral_max);
  int16_t power = ((int32_t) kP_speed * error + (int32_t) kI_speed * integral) >> 8;

  setPower(power);
}
