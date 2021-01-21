#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "Axis.h"

void setSpeedLeft(uint16_t speed, bool reverse);
void setSpeedRight(uint16_t speed, bool reverse);

static DualVNH5019MotorShield md;
static Axis axis_left(setSpeedLeft, true);
static Axis axis_right(setSpeedRight);

typedef enum {
  IDLE,
  MOVING
} state_t;

typedef struct {
  int16_t integral = 0;
  int32_t last_input = 0;
} pid_state_t;

volatile int32_t _encoder_left = 0;
volatile int32_t _encoder_right = 0;

ISR(PCINT2_vect) {
  // http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
  const int8_t _kEncoder_LUT[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
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
    "add %A[lut], %[state]           \n\t" // increment lut by state ie retrieve lut[state]
    "adc %B[lut], __zero_reg__       \n\t"
    "ld __tmp_reg__, Z               \n\t"
    /*
      add lut[state] to count
      - at this stage, __tmp_reg__ will contain either 0, 1, or -1 (0xFF)
      - adding 0 or 1 is straight forward, but to add 0xFF to int32_t, need to sign extend
        (ie add 0xFFFFFFFF)
      - as a cheap hack, we're going to clear __tmp_reg__ if the highest bit of it is clear (ie non negative),
        then add it to all the bytes of count. this allows us to continuously add __tmp_reg__ to all bytes to
        handle both 0, 1 and -1 without using 3 more registers
    */
    "add %A[count], __tmp_reg__      \n\t"
    "sbrs __tmp_reg__, 7             \n\t" // if highest bit is clear,
    "eor __tmp_reg__, __tmp_reg__    \n\t" //   clear __tmp_reg__ so adding it later dosent do anything
    "adc %B[count], __tmp_reg__      \n\t"
    "adc %C[count], __tmp_reg__      \n\t"
    "adc %D[count], __tmp_reg__      \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [count] "=w" (_encoder_left)
    :
      "0" (state),
      "1" (_encoder_left),
      [in] "I" (_SFR_IO_ADDR(E1_PIN)),
      [encA] "I" (E1A_BIT),
      [encB] "I" (E1B_BIT),
      [lut] "z" (_kEncoder_LUT)
  );
}

ISR(PCINT0_vect) {
  const int8_t _kEncoder_LUT[16] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
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
    "add %A[lut], %[state]           \n\t" // increment lut by state ie retrieve lut[state]
    "adc %B[lut], __zero_reg__       \n\t"
    "ld __tmp_reg__, Z               \n\t"
    /*
      add lut[state] to count
      - at this stage, __tmp_reg__ will contain either 0, 1, or -1 (0xFF)
      - adding 0 or 1 is straight forward, but to add 0xFF to int32_t, need to sign extend
        (ie add 0xFFFFFFFF)
      - as a cheap hack, we're going to clear __tmp_reg__ if the highest bit of it is clear (ie non negative),
        then add it to all the bytes of count. this allows us to continuously add __tmp_reg__ to all bytes to
        handle both 0, 1 and -1 without using 3 more registers
    */
    "add %A[count], __tmp_reg__      \n\t"
    "sbrs __tmp_reg__, 7             \n\t" // if highest bit is clear,
    "eor __tmp_reg__, __tmp_reg__    \n\t" //   clear __tmp_reg__ so adding it later dosent do anything
    "adc %B[count], __tmp_reg__      \n\t"
    "adc %C[count], __tmp_reg__      \n\t"
    "adc %D[count], __tmp_reg__      \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [count] "=w" (_encoder_right)
    :
      "0" (state),
      "1" (_encoder_right),
      [in] "I" (_SFR_IO_ADDR(E2_PIN)),
      [encA] "I" (E2A_BIT),
      [encB] "I" (E2B_BIT),
      [lut] "z" (_kEncoder_LUT)
  );
}

void setSpeedLeft(uint16_t speed, bool reverse) {
  OCR1A = speed;

  if (speed < kMin_motor_threshold) {
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

void setSpeedRight(uint16_t speed, bool reverse) {
  OCR1B = speed;

  if (speed < kMin_motor_threshold) {
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

int32_t error = 0;

pid_state_t state_tl;
int16_t controllerTrackLeft(int32_t encoder_left, int32_t encoder_right) {
  // controller aiming to make encoder_right track encoder_left
  // returns int16_t: positive - turn left, negative - turn right
  error = encoder_left - encoder_right;

  state_tl.integral = constrain(state_tl.integral + error, kTL_integral_min, kTL_integral_max);
  int16_t power = ((int32_t) kP_offset * error + (int32_t) kI_offset * state_tl.integral + (int32_t) kD_offset * (state_tl.last_input - encoder_right)) >> 8;

  state_tl.last_input = encoder_right;
  return power;
}

pid_state_t state_straight;
int16_t controllerStraight(int32_t encoder_left, int32_t target) {
  // controller aiming to make encoder_left track target
  int32_t error = target - encoder_left;

  state_straight.integral = constrain((int32_t) state_straight.integral + error, kMS_integral_min, kMS_integral_max);
  int16_t power = ((int32_t) kP_straight * error + (int32_t) kI_straight * state_straight.integral + (int32_t) kD_straight * (state_straight.last_input - encoder_left)) >> 8;

  state_straight.last_input = encoder_left;

  if (power > kMS_max_output) {
    return kMS_max_output;
  } else if (power < kMS_min_output) {
    return kMS_min_output;
  }

  return power;
}

static state_t state = IDLE;
static int32_t target_ticks = 0;

volatile int16_t base_power = 0;
volatile int16_t correction = 0;

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  static int32_t pEncoder_left = 0;
  static int32_t pEncoder_right = 0;
  static bool moved = false;

  if (state == IDLE) {
    axis_left.setSpeed(0);
    axis_right.setSpeed(0);
    return;
  }

  int32_t encoder_cor_left = axis_left.readEncoder(_encoder_left);
  int32_t encoder_cor_right = axis_right.readEncoder(_encoder_right);

  int32_t delta_left = encoder_cor_left - pEncoder_left;
  int32_t delta_right = encoder_cor_right - pEncoder_right;

  // whether the encoder has changed from the last update
  bool has_delta = (delta_left < -kEncoder_move_threshold) || (delta_left > kEncoder_move_threshold) ||
                  (delta_right < -kEncoder_move_threshold) || (delta_right > kEncoder_move_threshold);
  if (moved && !has_delta) {
    int32_t diff_left = encoder_cor_left - target_ticks;
    if (
      diff_left > -kMax_encoder_error && diff_left < kMax_encoder_error
    ) {
      Serial.println("move done");
      state = IDLE;
      moved = false;
      return;
    }
  } else if (!moved && has_delta) {
    Serial.println("move start");
    moved = true;
  }

  pEncoder_left = encoder_cor_left;
  pEncoder_right = encoder_cor_right;

  base_power = controllerStraight(encoder_cor_left, target_ticks);
  correction = controllerTrackLeft(encoder_cor_left, encoder_cor_right);

  int16_t power_left = base_power - correction;
  int16_t power_right = base_power + correction;

  axis_left.setSpeed(power_left);
  axis_right.setSpeed(power_right);
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

  md.init();
}

void start_motion(motion_direction_t _direction, uint32_t distance) {
  if (state != IDLE) {
    Serial.println("Cannot start motion, movement in progress");
    return;
  }

  cli();
  _encoder_left = 0;
  _encoder_right = 0;

  if (_direction == FORWARD) {
    state = MOVING;
    axis_left.setReverse(false);
    axis_right.setReverse(false);
    target_ticks = distance;
    Serial.print("start forward - target=");
    Serial.println(target_ticks);
  } else if (_direction == REVERSE) {
    state = MOVING;
    axis_left.setReverse(true);
    axis_right.setReverse(true);
    target_ticks = distance;
    Serial.print("start reverse - target=");
    Serial.println(target_ticks);
  } else if (_direction == LEFT) {
    state = MOVING;
    axis_left.setReverse(true);
    axis_right.setReverse(false);
    target_ticks = distance;
    Serial.print("start left turn - target=");
    Serial.println(target_ticks);
  } else if (_direction == RIGHT) {
    state = MOVING;
    axis_left.setReverse(false);
    axis_right.setReverse(true);
    target_ticks = distance;
    Serial.print("start right turn - target=");
    Serial.println(target_ticks);
  } else {
    Serial.println("Not implemented");
  }
  sei();
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
    int32_t __encoder_left = _encoder_left;
    int32_t __encoder_right = _encoder_right;
    int16_t speed_left = axis_left.getSpeed();
    int16_t speed_right = axis_right.getSpeed();
    int16_t _base_power = base_power;
    int16_t _correction = correction;
    int16_t _error = error;
    sei();
    Serial.print("SYNC");
    Serial.write((char *) &__encoder_left, 4);
    Serial.write((char *) &__encoder_right, 4);
    Serial.write((char *) &speed_left, 2);
    Serial.write((char *) &speed_right, 2);
    Serial.write((char *) &_error, 2);
    Serial.write((char *) &_correction, 2);
    Serial.println();

    last_print = cur_time;
  }
}
