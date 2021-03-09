#include <Arduino.h>
#include "motion.h"
#include "motion_init.h"
#include "board.h"
#include "config.h"

void setPowerLeft(uint16_t power, bool reverse);
void setPowerRight(uint16_t power, bool reverse);

Axis axis_left(setPowerLeft, true);
Axis axis_right(setPowerRight);

ISR(PCINT2_vect) {
  // http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
  // https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h
  static uint8_t state = 0;
  int8_t delta = 0;
  asm volatile(
      // update encoder state
      "lsl %[state]                    \n\t" // state << 2
      "lsl %[state]                    \n\t"
      "andi %[state], 0x0C             \n\t"
      "sbic %[in], %[encA]             \n\t" // if encoder A is set,
      "sbr %[state], (1<<0)            \n\t" //   set bit 0 of state
      "sbic %[in], %[encB]             \n\t" // if encoder B is set, 
      "sbr %[state], (1<<1)            \n\t" //   set bit 1 of state
      // use state to retrieve from LUT
      "ldi r30, lo8(pm(L%=jmptable))   \n\t"
      "ldi r31, hi8(pm(L%=jmptable))   \n\t"
      "add r30, %[state]               \n\t" // increment lut by state ie retrieve lut[state]
      "adc r31, __zero_reg__           \n\t"
      "ijmp                            \n\t"
    "L%=jmptable:                      \n\t"
      "rjmp L%=end                     \n\t" // 0
      "rjmp L%=plus1                   \n\t" // 1
      "rjmp L%=minus1                  \n\t" // 2
      "rjmp L%=end                     \n\t" // 3
      "rjmp L%=minus1                  \n\t" // 4
      "rjmp L%=end                     \n\t" // 5
      "rjmp L%=end                     \n\t" // 6
      "rjmp L%=plus1                   \n\t" // 7
      "rjmp L%=plus1                   \n\t" // 8
      "rjmp L%=end                     \n\t" // 9
      "rjmp L%=end                     \n\t" // 10
      "rjmp L%=minus1                  \n\t" // 11
      "rjmp L%=end                     \n\t" // 12
      "rjmp L%=minus1                  \n\t" // 13
      "rjmp L%=plus1                   \n\t" // 14
      "rjmp L%=end                     \n\t" // 15
    "L%=plus1:                         \n\t"
      "ldi %[delta], 1                 \n\t"
      "rjmp L%=end                     \n\t"
    "L%=minus1:                        \n\t"
      "ldi %[delta], 255               \n\t"
    "L%=end:                           \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [delta] "=d" (delta)
    :
      "0" (state),
      [in] "I" (_SFR_IO_ADDR(E1_PIN)),
      [encA] "I" (E1A_BIT),
      [encB] "I" (E1B_BIT)
    : "r30", "r31"
  );

  axis_left.encoderEdge(delta);
}

ISR(PCINT0_vect) {
  int8_t delta = 0;
  static uint8_t state = 0;
  asm volatile(
      // update encoder state
      "lsl %[state]                    \n\t" // state << 2
      "lsl %[state]                    \n\t"
      "andi %[state], 0x0C             \n\t"
      "sbic %[in], %[encA]             \n\t" // if encoder A is set,
      "sbr %[state], (1<<0)            \n\t" //   set bit 0 of state
      "sbic %[in], %[encB]             \n\t" // if encoder B is set, 
      "sbr %[state], (1<<1)            \n\t" //   set bit 1 of state
      // use state to retrieve from LUT
      "ldi r30, lo8(pm(L%=jmptable))   \n\t"
      "ldi r31, hi8(pm(L%=jmptable))   \n\t"
      "add r30, %[state]               \n\t" // increment lut by state ie retrieve lut[state]
      "adc r31, __zero_reg__           \n\t"
      "ijmp                            \n\t"
    "L%=jmptable:                      \n\t"
      "rjmp L%=end                     \n\t" // 0
      "rjmp L%=plus1                   \n\t" // 1
      "rjmp L%=minus1                  \n\t" // 2
      "rjmp L%=end                     \n\t" // 3
      "rjmp L%=minus1                  \n\t" // 4
      "rjmp L%=end                     \n\t" // 5
      "rjmp L%=end                     \n\t" // 6
      "rjmp L%=plus1                   \n\t" // 7
      "rjmp L%=plus1                   \n\t" // 8
      "rjmp L%=end                     \n\t" // 9
      "rjmp L%=end                     \n\t" // 10
      "rjmp L%=minus1                  \n\t" // 11
      "rjmp L%=end                     \n\t" // 12
      "rjmp L%=minus1                  \n\t" // 13
      "rjmp L%=plus1                   \n\t" // 14
      "rjmp L%=end                     \n\t" // 15
    "L%=plus1:                         \n\t"
      "ldi %A[delta], 1                 \n\t"
      "rjmp L%=end                     \n\t"
    "L%=minus1:                        \n\t"
      "ldi %A[delta], 255               \n\t"
    "L%=end:                           \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [delta] "=d" (delta)
    :
      "0" (state),
      [in] "I" (_SFR_IO_ADDR(E2_PIN)),
      [encA] "I" (E2A_BIT),
      [encB] "I" (E2B_BIT)
    : "r30", "r31"
  );

  axis_right.encoderEdge(delta);
}

void setPowerLeft(uint16_t power, bool reverse) {
  OCR1A = power;

  if (power < kMin_motor_threshold) {
    M1_INA_PORT &= ~_BV(M1_INA_BIT);
    M1_INB_PORT &= ~_BV(M1_INB_BIT);
  } else if (reverse) {
    M1_INA_PORT &= ~_BV(M1_INA_BIT);
    M1_INB_PORT |= _BV(M1_INB_BIT);
  } else {
    M1_INA_PORT |= _BV(M1_INA_BIT);
    M1_INB_PORT &= ~_BV(M1_INB_BIT);
  }
}

void setPowerRight(uint16_t power, bool reverse) {
  OCR1B = power;

  if (power < kMin_motor_threshold) {
    M2_INA_PORT &= ~_BV(M2_INA_BIT);
    M2_INB_PORT &= ~_BV(M2_INB_BIT);
  } else if (reverse) {
    M2_INA_PORT &= ~_BV(M2_INA_BIT);
    M2_INB_PORT |= _BV(M2_INB_BIT);
  } else {
    M2_INA_PORT |= _BV(M2_INA_BIT);
    M2_INB_PORT &= ~_BV(M2_INB_BIT);
  }
}

void setup_motion() {
  // PCI2 (left encoder):
  PCMSK2 |=  _BV(E1A_PCINT) | _BV(E1B_PCINT); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);                                       // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);                                       // enable PCI2

  // PCI0 (right encoder):
  PCMSK0 |=  _BV(E2A_PCINT) | _BV(E2B_PCINT); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);                                         // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);                                         // enable PCI0

  // configure timer 2 for 100Hz
  TCCR2A |= _BV(WGM21);                        // mode 2, CTC, top is OCRA
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20); // clock/1024
  OCR2A = 155;                                 // 16000000/1024/155 = 100Hz
  TIFR2 &= ~_BV(OCF2A);                        // clear interrupt flag
  TIMSK2 |= _BV(OCIE2A);                       // enable interrupt on compare match A

  // initialise motor shield pins
  pinMode(2, OUTPUT);  // _INA1
  pinMode(4, OUTPUT);  // _INB1
  pinMode(9, OUTPUT);  // _PWM1
  pinMode(7 ,OUTPUT);  // _INA2
  pinMode(8, OUTPUT);  // _INB2
  pinMode(10, OUTPUT); // _PWM2

  // Timer 1 configuration
  // prescaler: clockI/O / 1
  // outputs enabled
  // phase-correct PWM
  // top of 400
  //
  // PWM frequency calculation
  // 16MHz / 1 (prescaler) / 2 (phase-correct) / 400 (top) = 20kHz
  TCCR1A = 0b10100000;
  TCCR1B = 0b00010001;
  ICR1 = 400;
}
