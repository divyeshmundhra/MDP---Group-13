#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::encoderEdge() {
  _encoder_count ++;

  uint32_t time = micros();

  _pulse_width = ((uint32_t) (255 - kEncoder_alpha) * _pulse_width + (uint32_t) kEncoder_alpha * (time - _last_pulse)) >> 8;
  _last_pulse = time;
}

void Axis::controller() {
  cli();
  uint16_t __pulse_width = _pulse_width;
  uint32_t __last_pulse = _last_pulse;
  _encoder_count = 0;
  sei();

  if ((micros() - __last_pulse) > kEncoder_timeout) {
    // if last encoder pulse is too old, treat the motor as not spinning
    // blip speed to max and reset integral state
    _power = 400;
    _integral = 0;
  } else {
    // actual PID controller
    _last_error = _error;
    _error = __pulse_width - 1000;

    _integral = constrain(_integral + _error, kPID_integral_min, kPID_integral_max);

    _power = (kP_left * _error + kI_left * _integral + kD_left * (_last_error - _error)) >> 8;
  }
}
