#include <Arduino.h>

#include "board.h"
#include "motion.h"
#include "sensors.h"

void setup() {
  // setup_motion();
  setup_sensors();

  sei();

  Serial.begin(115200);
}

void loop() {
  // loop_motion();
  loop_sensors();
}
