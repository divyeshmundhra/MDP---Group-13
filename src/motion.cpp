#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "Axis.h"

DualVNH5019MotorShield md;
Axis axis_left;
Axis axis_right;

ISR(PCINT2_vect) {
  axis_left.encoderEdge();
}

ISR(PCINT0_vect) {
  axis_right.encoderEdge();
}

const uint16_t kP_offset = 500;
const uint16_t kI_offset = 0;
const uint16_t kD_offset = 0;

int16_t controllerTrackLeft(uint32_t encoder_left, uint32_t encoder_right) {
  // PID controller aiming to make encoder_right track encoder_left
  // returns int16_t: positive - turn left, negative - turn right
  static int16_t integral = 0;
  static int16_t last_error = 0;
  int32_t error = encoder_left - encoder_right;
  int16_t power = (kP_offset * error + kI_offset * integral + kD_offset * (last_error - error)) >> 8;
  integral = constrain(integral + error, kPID_integral_min, kPID_integral_max);
  last_error = error;

  return power;
}

const uint16_t kP_straight = 255;

uint16_t controllerStraight(uint32_t encoder_left, uint32_t target) {
  // P controller aiming to make encoder_left track target
  if (encoder_left >= target) {
    return 0;
  }

  uint32_t error = target - encoder_left;
  return (kP_straight * error) >> 8;
}

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  cli();
  uint32_t encoder_left = axis_left.getEncoderCount();
  uint32_t encoder_right = axis_right.getEncoderCount();
  sei();

  uint16_t base_power = controllerStraight(encoder_left, 5000);
  int16_t correction = controllerTrackLeft(encoder_left, encoder_right);

  md.setM1Speed(base_power - correction);
  md.setM2Speed(-(base_power + correction));

  Serial.print("SYNC");
  Serial.write((char *) &encoder_left, 4);
  Serial.write((char *) &encoder_right, 4);
  Serial.write((char *) &base_power, 2);
}

void setup_motion() {
  // PCI2 (left encoder):
  PCMSK2 |=  _BV(PIN_ENCODER_LEFT1) | _BV(PIN_ENCODER_LEFT2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);                                       // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);                                       // enable PCI2

  // PCI0 (right encoder):
  PCMSK0 |=  _BV(PIN_ENCODER_RIGHT1) | _BV(PIN_ENCODER_RIGHT2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);                                         // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);                                         // enable PCI0

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
    delay(1000);
    pinMode(PIN_MD_EN, INPUT_PULLUP);
  } else {
    // pinMode(PIN_MD_EN, INPUT);
  }
}
