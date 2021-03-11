#include <stdint.h>
#include "physical.h"

int32_t unit_turn_to_ticks(uint16_t units) {
  if (units == 2) {
    return kTicks_per_90_degrees;
  } else if (units == 4) {
    return kTicks_per_180_degrees;
  } else {
    return units * kTicks_per_45_degrees;
  }
}
