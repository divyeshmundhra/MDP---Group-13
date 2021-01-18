#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "Axis.h"

void setSpeedLeft(uint16_t speed, uint8_t fwd);
void setSpeedRight(uint16_t speed, uint8_t fwd);

static DualVNH5019MotorShield md;
static Axis axis_left(setSpeedLeft);
static Axis axis_right(setSpeedRight, true);

typedef enum {
  IDLE,
  MOVING
} state_t;

ISR(PCINT2_vect) {
  axis_left.encoderEdge();
}

ISR(PCINT0_vect) {
  axis_right.encoderEdge();
}

void setSpeedLeft(uint16_t speed, uint8_t fwd) {
  OCR1A = speed;

  if (speed < kMin_motor_threshold) {
    M1_INA_PORT &= ~_BV(M1_INA_BIT);
    M1_INB_PORT &= ~_BV(M1_INB_BIT);
  } else if (fwd) {
    M1_INA_PORT |= _BV(M1_INA_BIT);
    M1_INB_PORT &= ~_BV(M1_INB_BIT);
  } else {
    M1_INA_PORT &= ~_BV(M1_INA_BIT);
    M1_INB_PORT |= _BV(M1_INB_BIT);
  }
}

void setSpeedRight(uint16_t speed, uint8_t fwd) {
  OCR1B = speed;

  if (speed < kMin_motor_threshold) {
    M2_INA_PORT &= ~_BV(M2_INA_BIT);
    M2_INB_PORT &= ~_BV(M2_INB_BIT);
  } else if (fwd) {
    M2_INA_PORT |= _BV(M2_INA_BIT);
    M2_INB_PORT &= ~_BV(M2_INB_BIT);
  } else {
    M2_INA_PORT &= ~_BV(M2_INA_BIT);
    M2_INB_PORT |= _BV(M2_INB_BIT);
  }
}

int16_t controllerTrackLeft(uint32_t encoder_left, uint32_t encoder_right) {
  // PID controller aiming to make encoder_right track encoder_left
  // returns int16_t: positive - turn left, negative - turn right
  static int16_t integral = 0;
  static int16_t last_error = 0;
  int32_t error = encoder_left - encoder_right;

  integral = constrain(integral + error, kPID_integral_min, kPID_integral_max);
  int16_t power = (kP_offset * error + kI_offset * integral + kD_offset * (last_error - error)) >> 8;

  last_error = error;
  return power;
}

uint16_t controllerStraight(uint32_t encoder_left, uint32_t target) {
  // P controller aiming to make encoder_left track target
  if (encoder_left >= target) {
    return 0;
  }

  uint32_t error = target - encoder_left;
  return (kP_straight * error) >> 8;
}

static state_t state = IDLE;
static motion_direction_t direction;
static uint32_t target_ticks = 0;

volatile uint16_t base_power = 0;
volatile int16_t correction = 0;

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  if (state == IDLE) {
    return;
  }

  cli();
  uint32_t encoder_left = axis_left.getEncoderCount();
  uint32_t encoder_right = axis_right.getEncoderCount();
  sei();

  if (direction == FORWARD) {
    base_power = controllerStraight(encoder_left, target_ticks);
    correction = controllerTrackLeft(encoder_left, encoder_right);

    if (base_power == 0 && correction > -10 && correction < 10) {
      Serial.println("done");
      state = IDLE;
    }

    int16_t power_left = base_power - correction;
    int16_t power_right = base_power + correction;

    axis_left.setTargetSpeed(power_left);
    axis_right.setTargetSpeed(power_right);
  }
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
}

void start_motion(motion_direction_t _direction, uint32_t distance) {
  if (state != IDLE) {
    Serial.println("Cannot start motion, movement in progress");
    return;
  }

  if (_direction == FORWARD) {
    cli();
    state = MOVING;
    direction = _direction;
    target_ticks = axis_left.getEncoderCount() + distance;
    Serial.print(target_ticks);
    Serial.print(", ");
    Serial.print(distance);
    Serial.println("start forward");
    sei();
  } else {
    Serial.println("Not implemented");
  }
}

void loop_motion() {
  static state_t pState = IDLE;

  if (pState != state) {
    if (state == IDLE) {
      Serial.println("done");
    }
  }

  static uint32_t last_print = 0;
  uint32_t cur_time = millis();

  if ((cur_time - last_print) > 10) {
    cli();
    uint32_t encoder_left = axis_left.getEncoderCount();
    uint32_t encoder_right = axis_right.getEncoderCount();
    int16_t speed_left = axis_left.getCurSpeed();
    int16_t speed_right = axis_right.getCurSpeed();
    int16_t _base_power = base_power;
    int16_t _correction = correction;
    sei();
    Serial.print("SYNC");
    Serial.write((char *) &encoder_left, 4);
    Serial.write((char *) &encoder_right, 4);
    Serial.write((char *) &speed_left, 2);
    Serial.write((char *) &speed_right, 2);
    Serial.write((char *) &_base_power, 2);
    Serial.write((char *) &_correction, 2);
    Serial.println();

    last_print = cur_time;
  }
}
