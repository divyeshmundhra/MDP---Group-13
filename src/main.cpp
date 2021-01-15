#include <Arduino.h>

#include "board.h"
#include "motion.h"

void setup() {
  setup_motion();

  sei();

  Serial.begin(115200);
}

void loop() {
  loop_motion();
}
