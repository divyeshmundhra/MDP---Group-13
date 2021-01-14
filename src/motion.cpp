#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "Axis.h"

DualVNH5019MotorShield md;
Axis axis_left(kP_left, kI_left, kD_left);
Axis axis_right(kP_right, kI_right, kD_right);

ISR(PCINT2_vect) {
  axis_left.encoderEdge();
}

ISR(PCINT0_vect) {
  axis_right.encoderEdge();
}

ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  axis_left.controller();
  axis_right.controller();

  md.setM1Speed(axis_left.getPower());
  md.setM2Speed(axis_right.getPower());

  cli();
  int16_t _power_left = axis_left.getPower();
  int16_t _width_left = axis_left.getPulseWidth();
  int16_t _width_right = axis_right.getPulseWidth();
  sei();

  Serial.print("SYNC");
  Serial.write((char *) &_power_left, 2);
  Serial.write((char *) &_width_left, 2);
  Serial.write((char *) &_width_right, 2);
}

void setup_motion() {
  // PCI2 (left encoder):
  PCMSK2 |=  _BV(PIN_ENCODER_LEFT1); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);              // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);              // enable PCI2

  // PCI0 (right encoder):
  PCMSK0 |=  _BV(PIN_ENCODER_RIGHT1); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);             // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);             // enable PCI0

  // configure timer 2 for 100Hz
  TCCR2A |= _BV(WGM21);                        // mode 2, CTC, top is OCRA
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20); // clock/1024
  OCR2A = 155;                                 // 16000000/1024/155 = 100Hz
  TIFR2 &= ~_BV(OCF2A);                        // clear interrupt flag
  TIMSK2 |= _BV(OCIE2A);                       // enable interrupt on compare match A

  md.init();

  pinMode(PIN_SW, INPUT_PULLUP);
}

void loop_motion() {
  if (!digitalRead(PIN_SW)) {
    pinMode(PIN_MD_EN, INPUT_PULLUP);
  } else {
    pinMode(PIN_MD_EN, INPUT);
  }
}
