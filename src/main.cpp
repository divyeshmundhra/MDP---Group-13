#include <Arduino.h>
#include "DualVNH5019MotorShield.h"

// both assumed to be in PCI2
#define PIN_ENCODER_A1 PCINT19
#define PIN_ENCODER_A2 PCINT21

// both assumed to be in PCI0
#define PIN_ENCODER_B1 PCINT3
#define PIN_ENCODER_B2 PCINT5

#define PIN_MD_EN 6
#define PIN_SW 14

const int32_t kP_left = 535;
const int32_t kI_left = 28;
const int32_t kD_left = 383;

// min/max of integral state
const int16_t kPID_integral_min = -2000;
const int16_t kPID_integral_max = 2000;

// alpha for encoder filtering
// smaller for more aggressive filtering
const uint8_t kEncoder_alpha = 25;

// timeout (us) for time since encoder pulse received
// after which a motor is treated as stationary
const uint16_t kEncoder_timeout = 10000;

DualVNH5019MotorShield md;

volatile uint16_t count_left = 0;
volatile uint16_t count_right = 0;

volatile uint32_t last_pulse_time_left = 0;
volatile uint16_t width_left = 0;

ISR(PCINT2_vect) {
  count_left ++;

  uint32_t time = micros();

  width_left = ((uint32_t) (255 - kEncoder_alpha) * width_left + (uint32_t) kEncoder_alpha * (time - last_pulse_time_left)) >> 8;
  last_pulse_time_left = time;
}

ISR(PCINT0_vect) {
  count_right ++;
}

int16_t integral_left = 0;
int16_t last_error_left = 0;

int16_t error_left, speed_left;

ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  cli();
  uint16_t _width_left = width_left;
  uint32_t _last_pulse_time_left = last_pulse_time_left;
  count_left = 0;
  count_right = 0;
  sei();

  if ((micros() - _last_pulse_time_left) > kEncoder_timeout) {
    // if last encoder pulse is too old, treat the motor as not spinning
    // blip speed to max and reset integral state
    speed_left = 400;
    integral_left = 0;
  } else {
    // actual PID controller
    error_left = _width_left - 1000;

    integral_left += error_left;
    integral_left = constrain(integral_left, kPID_integral_min, kPID_integral_max);

    speed_left = (kP_left * error_left + kI_left * integral_left + kD_left * (last_error_left - error_left)) >> 8;
  }

  md.setM1Speed(speed_left);
}

void setup() {
  // PCI2 (Motor A encoder):
  PCMSK2 |=  _BV(PIN_ENCODER_A1); // | _BV(PIN_ENCODER_A2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);                                // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);                                // enable PCI2

  // PCI0 (Motor B encoder):
  PCMSK0 |=  _BV(PIN_ENCODER_B1) | _BV(PIN_ENCODER_B2); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);                                // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);                                // enable PCI0

  // Configure timer 2 for 100Hz
  TCCR2A |= _BV(WGM21);                        // mode 2, CTC, top is OCRA
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20); // clock/1024
  OCR2A = 155;                                 // 16000000/1024/155 = 100Hz
  TIFR2 &= ~_BV(OCF2A);                        // clear interrupt flag
  TIMSK2 |= _BV(OCIE2A);                       // enable interrupt on compare match A

  md.init();

  pinMode(PIN_SW, INPUT_PULLUP);
  // md.setM1Speed(200);

  Serial.begin(115200);
  sei();
}

void loop() {
  static uint32_t last_print = millis();

  uint32_t cur_time = millis();

  if (!digitalRead(PIN_SW)) {
    pinMode(PIN_MD_EN, INPUT_PULLUP);
  } else {
    pinMode(PIN_MD_EN, INPUT);
  }

  if ((cur_time - last_print) > 10) {
    cli();
    int16_t _speed_left = speed_left;
    int16_t _error_left = error_left;
    int16_t _width_left = width_left;
    sei();

    Serial.print("START");
    Serial.write((char *) &_speed_left, 2);
    Serial.write((char *) &_error_left, 2);
    Serial.write((char *) &_width_left, 2);
    Serial.println();

    last_print = cur_time;
  }
}
