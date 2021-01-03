#include <Arduino.h>
#include "DualVNH5019MotorShield.h"

// both assumed to be in PCI2
#define PIN_ENCODER_A1 PCINT19
#define PIN_ENCODER_A2 PCINT21

// both assumed to be in PCI0
#define PIN_ENCODER_B1 PCINT3
#define PIN_ENCODER_B2 PCINT5

#define PIN_MD_EN 6

DualVNH5019MotorShield md;

volatile uint16_t count_left = 0;
volatile uint16_t count_right = 0;

ISR(PCINT2_vect) {
  count_left ++;
}

ISR(PCINT0_vect) {
  count_right ++;
}

void setup() {
  // PCI2 (Motor A encoder):
  PCMSK2 |=  _BV(PIN_ENCODER_A1) | _BV(PIN_ENCODER_A2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);                                // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);                                // enable PCI2

  // PCI0 (Motor B encoder):
  PCMSK0 |=  _BV(PIN_ENCODER_B1) | _BV(PIN_ENCODER_B2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);                                // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);                                // enable PCI0

  md.init();
  pinMode(PIN_MD_EN, INPUT_PULLUP);

  Serial.begin(115200);
}

void loop() {
  static uint32_t last_print = millis();
  uint32_t cur_time = millis();

  if ((cur_time - last_print) > 100) {
    cli();
    Serial.print(count_left);
    Serial.print(", ");
    Serial.println(count_right);

    count_left = 0;
    count_right = 0;

    sei();
    last_print = cur_time;
  }
}
