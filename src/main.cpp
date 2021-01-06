#include <Arduino.h>

#include "board.h"
#include "motion.h"

void setup() {
  TIME_DDR |= _BV(TIME_BIT);
  setup_motion();

  sei();

  Serial.begin(115200);
}

void loop() {
  loop_motion();
}
