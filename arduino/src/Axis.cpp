#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::setPower(int16_t target_power) {
  if (target_power > 0) {
    _setPower(target_power, _invert ^ _reverse);
  } else {
    _setPower(-target_power, !(_invert ^ _reverse));
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
