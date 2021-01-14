#include <Arduino.h>

#include "Axis.h"
#include "config.h"

void Axis::encoderEdge() {
  _encoder_count ++;
}
