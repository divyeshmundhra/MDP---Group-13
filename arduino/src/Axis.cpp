#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::setSpeed(int16_t speed) {
  _speed = speed;

  if (_speed > 0) {
    _setSpeed(_speed, !_reverse);
  } else {
    _setSpeed(-_speed, _reverse);
  }
}
