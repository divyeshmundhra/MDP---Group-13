#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "sensors.h"
#include "Axis.h"

void setPowerLeft(uint16_t power, bool reverse);
void setPowerRight(uint16_t power, bool reverse);

static DualVNH5019MotorShield md;
static Axis axis_left(setPowerLeft, true);
static Axis axis_right(setPowerRight);

typedef enum {
  IDLE,
  MOVE_COMMANDED,
  MOVING
} state_t;

typedef enum {
  DISTANCE,
  OBSTACLE
} move_type_t;

typedef struct {
  int16_t integral = 0;
  int32_t last_input = 0;
} pid_state_t;

volatile int32_t _encoder_left = 0;
volatile int32_t _encoder_right = 0;

ISR(PCINT2_vect) {
  // http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
  const int8_t _kEncoder_LUT[16] = {0, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 0};
  static uint8_t state = 0;

  asm volatile(
    // update encoder state
    "lsl %[state]                    \n\t" // state << 2
    "lsl %[state]                    \n\t"
    "andi %[state], 0x0C             \n\t"
    "sbic %[in], %[encA]             \n\t" // if encoder A is set,
    "sbr %[state], (1<<1)            \n\t" //   set bit 1 of state
    "sbic %[in], %[encB]             \n\t" // if encoder B is set, 
    "sbr %[state], (1<<0)            \n\t" //   set bit 0 of state
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

  axis_left.encoderEdge();
}

ISR(PCINT0_vect) {
  const int8_t _kEncoder_LUT[16] = {0, 0, 0, -1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, 0};
  static uint8_t state = 0;

  asm volatile(
    // update encoder state
    "lsl %[state]                    \n\t" // state << 2
    "lsl %[state]                    \n\t"
    "andi %[state], 0x0C             \n\t"
    "sbic %[in], %[encA]             \n\t" // if encoder A is set,
    "sbr %[state], (1<<1)            \n\t" //   set bit 1 of state
    "sbic %[in], %[encB]             \n\t" // if encoder B is set, 
    "sbr %[state], (1<<0)            \n\t" //   set bit 0 of state
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

  axis_right.encoderEdge();
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

pid_state_t state_tl;
void resetControllerState(pid_state_t *state, int32_t input) {
  state->integral = 0;
  state->last_input = input;
}

int16_t controllerTrackLeft(int32_t encoder_left, int32_t encoder_right) {
  // controller aiming to make encoder_right track encoder_left
  // returns int16_t: positive - turn left, negative - turn right
  int32_t error = encoder_left - encoder_right;

  state_tl.integral = constrain(state_tl.integral + error, kTL_integral_min, kTL_integral_max);
  int16_t power = ((int32_t) kP_offset * error + (int32_t) kI_offset * state_tl.integral + (int32_t) kD_offset * (state_tl.last_input - encoder_right)) >> 8;

  state_tl.last_input = encoder_right;
  return power;
}

pid_state_t state_straight_left;
pid_state_t state_straight_right;
int16_t controllerStraight(pid_state_t *state, int32_t encoder_left, int32_t target) {
  // controller aiming to make encoder_left track target
  int32_t error = target - encoder_left;

  state->integral = constrain((int32_t) state->integral + error, kMS_integral_min, kMS_integral_max);
  int16_t power = ((int32_t) kP_straight * error + (int32_t) kI_straight * state->integral + (int32_t) kD_straight * (state->last_input - encoder_left)) >> 8;

  state->last_input = encoder_left;

  if (power > kMS_max_output) {
    return kMS_max_output;
  } else if (power < kMS_min_output) {
    return kMS_min_output;
  }

  return power;
}

pid_state_t state_obstacle;
int16_t controllerObstacle(pid_state_t *state, uint16_t distance, uint16_t target) {
  int16_t error = distance - target;

  state->integral = constrain((int32_t) state->integral + error, kMO_integral_min, kMO_integral_max);
  int16_t power = ((int32_t) kP_obstacle * error + (int32_t) kI_obstacle * state->integral + (int32_t) kD_obstacle * (state->last_input - distance)) >> 8;

  state->last_input = distance;

  if (power > kMO_max_output) {
    return kMO_max_output;
  } else if (power < kMO_min_output) {
    return kMO_min_output;
  }

  return power;
}

static state_t state = IDLE;
static move_type_t move_type = DISTANCE;
// depending on move_type, stores either target encoder ticks or target obstacle distance
static int32_t target = 0;

volatile int16_t base_left = 0;
volatile int16_t base_right = 0;
volatile int16_t correction = 0;

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  static int32_t pEncoder_left = 0;
  static int32_t pEncoder_right = 0;

  convert_sensor_data();
  axis_left.controllerSpeed();
  axis_right.controllerSpeed();

  // if (state == IDLE) {
  //   axis_left.setPower(0);
  //   axis_right.setPower(0);
  //   return;
  // }

  // int32_t encoder_cor_left = axis_left.readEncoder(_encoder_left);
  // int32_t encoder_cor_right = axis_right.readEncoder(_encoder_right);

  // int32_t delta_left = encoder_cor_left - pEncoder_left;
  // int32_t delta_right = encoder_cor_right - pEncoder_right;

  // // whether the encoder has changed from the last update
  // bool has_delta = (delta_left < -kEncoder_move_threshold) || (delta_left > kEncoder_move_threshold) ||
  //                 (delta_right < -kEncoder_move_threshold) || (delta_right > kEncoder_move_threshold);

  // if (state == MOVING && !has_delta) {
  //   // encoder delta has slowed to almost zero - lets check if we should finish the move
  //   int32_t diff_err = encoder_cor_left - encoder_cor_right;
  //   if (diff_err > -kMax_encoder_diff_error && diff_err < kMax_encoder_diff_error) {
  //     // if the error between both encoders are really similar, we can check for the
  //     // move-specific terminating condition
  //     if (move_type == DISTANCE) {
  //       // DISTANCE terminates when the left encoder is really close to target
  //       int32_t diff_left = encoder_cor_left - target;
  //       if (diff_left > -kMax_encoder_error && diff_left < kMax_encoder_error) {
  //         Serial.println("move distance done");
  //         state = IDLE;
  //         return;
  //       }
  //     } else if (move_type == OBSTACLE) {
  //       // OBSTACLE terminates when the sensor distance is really close to target
  //       int16_t diff_err = sensor_distances[FRONT_MID] - target;
  //       if (diff_err > -kMax_obstacle_error && diff_err < kMax_obstacle_error) {
  //         Serial.println("move obstacle done");
  //         state = IDLE;
  //         return;
  //       }
  //     }
  //   }
  // } else if (state == MOVE_COMMANDED) {
  //   resetControllerState(&state_tl, encoder_cor_right);
  //   if (move_type == DISTANCE) {
  //     resetControllerState(&state_straight_left, encoder_cor_left);
  //     resetControllerState(&state_straight_right, encoder_cor_right);
  //   } else if (move_type == OBSTACLE) {
  //     resetControllerState(&state_obstacle, sensor_distances[FRONT_MID]);
  //   }
  //   state = MOVING;
  // }

  // pEncoder_left = encoder_cor_left;
  // pEncoder_right = encoder_cor_right;

  // if (move_type == DISTANCE) {
  //   base_left = controllerStraight(&state_straight_left, encoder_cor_left, target);
  //   base_right = controllerStraight(&state_straight_right, encoder_cor_right, target);
  // } else if (move_type == OBSTACLE) {
  //   base_left = controllerObstacle(&state_obstacle, sensor_distances[FRONT_MID], target);
  //   base_right = base_left;
  // }

  // correction = controllerTrackLeft(encoder_cor_left, encoder_cor_right);

  // int16_t power_left = base_left - correction;
  // int16_t power_right = base_right + correction;

  // axis_left.setPower(power_left);
  // axis_right.setPower(power_right);
}

bool get_motion_done() {
  return state == IDLE;
}

int32_t get_encoder_left() {
  return axis_left.readEncoder(_encoder_left);
}

void setup_motion() {
  // PCI2 (left encoder):
  PCMSK2 |=  _BV(E1A_PCINT); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF2);     // clear interrupt flag of PCI2
  PCICR  |=  _BV(PCIE2);     // enable PCI2

  // PCI0 (right encoder):
  PCMSK0 |=  _BV(E2A_PCINT); // enable interrupt sources
  PCIFR  &= ~_BV(PCIF0);     // clear interrupt flag of PCI0
  PCICR  |=  _BV(PCIE0);     // enable PCI0

  // configure timer 2 for 100Hz
  TCCR2A |= _BV(WGM21);                        // mode 2, CTC, top is OCRA
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20); // clock/1024
  OCR2A = 155;                                 // 16000000/1024/155 = 100Hz
  TIFR2 &= ~_BV(OCF2A);                        // clear interrupt flag
  TIMSK2 |= _BV(OCIE2A);                       // enable interrupt on compare match A

  md.init();
}

void start_motion_distance(motion_direction_t _direction, uint32_t distance) {
  if (state != IDLE) {
    Serial.println("Cannot start motion, movement in progress");
    return;
  }

  cli();
  _encoder_left = 0;
  _encoder_right = 0;

  state = MOVE_COMMANDED;
  move_type = DISTANCE;
  target = distance;

  if (_direction == FORWARD) {
    axis_left.setReverse(false);
    axis_right.setReverse(false);
    Serial.print("start forward - target=");
    Serial.println(target);
  } else if (_direction == REVERSE) {
    axis_left.setReverse(true);
    axis_right.setReverse(true);
    Serial.print("start reverse - target=");
    Serial.println(target);
  } else if (_direction == LEFT) {
    axis_left.setReverse(true);
    axis_right.setReverse(false);
    Serial.print("start left turn - target=");
    Serial.println(target);
  } else if (_direction == RIGHT) {
    axis_left.setReverse(false);
    axis_right.setReverse(true);
    Serial.print("start right turn - target=");
    Serial.println(target);
  } else {
    Serial.println("Not implemented");
  }
  sei();
}

void start_motion_obstacle(uint16_t distance) {
  if (state != IDLE) {
    Serial.println("Cannot start motion, movement in progress");
    return;
  }

  cli();
  _encoder_left = 0;
  _encoder_right = 0;

  state = MOVE_COMMANDED;
  move_type = OBSTACLE;
  target = distance;
  axis_left.setReverse(false);
  axis_right.setReverse(false);
  sei();
}

void set_speed(uint16_t speed) {
  axis_left.setTargetSpeed(speed);
  axis_right.setTargetSpeed(speed);
}

void loop_motion() {
  static state_t pState = IDLE;

  if (pState != state) {
    if (state == IDLE) {
      Serial.println("done");
    }
  }

  #if 1
    static uint32_t last_print = 0;
    uint32_t cur_time = millis();

    if ((cur_time - last_print) > 10) {
      cli();
      int32_t __encoder_left = _encoder_left;
      int32_t __encoder_right = _encoder_right;
      int16_t power_left = axis_left.getPower();
      int16_t power_right = axis_right.getPower();
      // int16_t _base_power = base_power;
      int16_t _correction = correction;
      int16_t _error = __encoder_left - __encoder_right;
      uint16_t _sensor_mid = sensor_distances[FRONT_MID];
      uint16_t speed_left = axis_left.getSpeed();
      uint16_t speed_right = axis_right.getSpeed();
      sei();
      Serial.print("SYNC");
      Serial.write((char *) &__encoder_left, 4);
      Serial.write((char *) &__encoder_right, 4);
      Serial.write((char *) &power_left, 2);
      Serial.write((char *) &power_right, 2);
      Serial.write((char *) &speed_left, 2);
      Serial.write((char *) &speed_right, 2);
      // Serial.write((char *) &_error, 2);
      // Serial.write((char *) &_correction, 2);
      // Serial.write((char *) &_sensor_mid, 2);
      Serial.println();

      last_print = cur_time;
    }
  #endif
}
