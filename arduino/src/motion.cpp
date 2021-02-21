#include <Arduino.h>

#include "DualVNH5019MotorShield.h"

#include "motion.h"
#include "config.h"
#include "board.h"
#include "sensors.h"
#include "physical.h"
#include "align_LUT.h"
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
  move_type_t type;
  motion_direction_t direction;
  int32_t target;
} move_t;

typedef struct {
  int16_t integral = 0;
  int32_t last_input = 0;
} pid_state_t;

ISR(PCINT2_vect) {
  // http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
  // https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h
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
      "subi %A[count], 255             \n\t"
      "sbci %B[count], 255             \n\t"
      "sbci %C[count], 255             \n\t"
      "sbci %D[count], 255             \n\t"
      "rjmp L%=end                     \n\t"
    "L%=minus1:                        \n\t"
      "subi %A[count], 1               \n\t"
      "sbci %B[count], 0               \n\t"
      "sbci %C[count], 0               \n\t"
      "sbci %D[count], 0               \n\t"
    "L%=end:                           \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [count] "=w" (axis_left.encoder_count)
    :
      "0" (state),
      "1" (axis_left.encoder_count),
      [in] "I" (_SFR_IO_ADDR(E1_PIN)),
      [encA] "I" (E1A_BIT),
      [encB] "I" (E1B_BIT)
    : "r30", "r31"
  );
}

ISR(PCINT0_vect) {
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
      "subi %A[count], 255             \n\t"
      "sbci %B[count], 255             \n\t"
      "sbci %C[count], 255             \n\t"
      "sbci %D[count], 255             \n\t"
      "rjmp L%=end                     \n\t"
    "L%=minus1:                        \n\t"
      "subi %A[count], 1               \n\t"
      "sbci %B[count], 0               \n\t"
      "sbci %C[count], 0               \n\t"
      "sbci %D[count], 0               \n\t"
    "L%=end:                           \n\t"
    :
      [state] "=d" (state), // has to go into upper register because ANDI and SBR only operate on upper
      [count] "=w" (axis_right.encoder_count)
    :
      "0" (state),
      "1" (axis_right.encoder_count),
      [in] "I" (_SFR_IO_ADDR(E2_PIN)),
      [encA] "I" (E2A_BIT),
      [encB] "I" (E2B_BIT)
    : "r30", "r31"
  );
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

  if (state == IDLE) {
    axis_left.setPower(0);
    axis_right.setPower(0);
    return;
  }

  int32_t encoder_left = axis_left.getEncoder();
  int32_t encoder_right = axis_right.getEncoder();

  int32_t delta_left = encoder_left - pEncoder_left;
  int32_t delta_right = encoder_right - pEncoder_right;

  pEncoder_left = encoder_left;
  pEncoder_right = encoder_right;

  // whether the encoder has changed from the last update
  bool has_encoder_delta = (delta_left < -kEncoder_move_threshold) || (delta_left > kEncoder_move_threshold) ||
                  (delta_right < -kEncoder_move_threshold) || (delta_right > kEncoder_move_threshold);

  // every recalculation of report_dist will start from base
  static uint16_t next_report_dist_base;
  // actual may have the report offset summed into it
  static uint16_t next_report_dist_actual;

  if (state == MOVING && !has_encoder_delta) {
    // encoder delta has slowed to almost zero - lets check if we should finish the move
    int32_t diff_err = encoder_left - encoder_right;
    if (diff_err > -kMax_encoder_diff_error && diff_err < kMax_encoder_diff_error) {
      // if the error between both encoders are really similar, we can check for the
      // move-specific terminating condition
      if (move_type == DISTANCE) {
        // DISTANCE terminates when the left encoder is really close to target
        int32_t diff_left = encoder_left - target;
        if (diff_left > -kMax_encoder_error && diff_left < kMax_encoder_error) {
          Serial.println("move distance done");
          state = IDLE;
          axis_left.setPower(0);
          axis_right.setPower(0);
          return;
        }
      } else if (move_type == OBSTACLE) {
        // OBSTACLE terminates when the sensor distance is really close to target
        int16_t diff_err = sensor_distances[FRONT_FRONT_MID] - target;
        if (diff_err > -kMax_obstacle_error && diff_err < kMax_obstacle_error) {
          Serial.println("move obstacle done");
          state = IDLE;
          axis_left.setPower(0);
          axis_right.setPower(0);
          return;
        }
      }
    }
  } else if (state == MOVE_COMMANDED) {
    resetControllerState(&state_tl, encoder_right);
    if (move_type == DISTANCE) {
      resetControllerState(&state_straight_left, encoder_left);
      resetControllerState(&state_straight_right, encoder_right);
    } else if (move_type == OBSTACLE) {
      resetControllerState(&state_obstacle, sensor_distances[FRONT_FRONT_MID]);
    }
    state = MOVING;
    next_report_dist_base = 0;
  }

  int16_t power_left = 0, power_right = 0;
  if (move_type == DISTANCE) {
    base_left = controllerStraight(&state_straight_left, encoder_left, target);
    base_right = controllerStraight(&state_straight_right, encoder_right, target);
  } else if (move_type == OBSTACLE) {
    base_left = controllerObstacle(&state_obstacle, sensor_distances[FRONT_FRONT_MID], target);
    base_right = base_left;
  }

  correction = controllerTrackLeft(encoder_left, encoder_right);

  power_left = base_left - correction;
  power_right = base_right + correction;

  axis_left.setPower(power_left);
  axis_right.setPower(power_right);

  // we treat dist_base == 0 as a "initialise the value" condition
  // so that we don't have to duplicate the logic for incrementing dist_actual into the MOVE_COMMANDED handler
  if (encoder_left > next_report_dist_actual || next_report_dist_base == 0) {
    if (next_report_dist_base > 0) {
      log_all_sensors();
      next_report_dist_base += kBlock_distance;
    } else {
      next_report_dist_base = kReport_distance;
    }

    // if we will eventually travel fully past this block, delay logging the sensors
    // until we move past it by kReport_distance_offset to get a more accurate reading
    uint16_t offset_target = next_report_dist_base + kReport_distance_offset;

    if (offset_target < target) {
      next_report_dist_actual = offset_target;
    } else {
      next_report_dist_actual = next_report_dist_base;
    }
  }
}

bool get_motion_done() {
  return state == IDLE;
}

int32_t get_encoder_left() {
  return axis_left.getEncoder();
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

move_t buffered_moves[kMovement_buffer_size];
uint8_t num_moves = 0;
uint8_t pos_moves_start = 0;
uint8_t pos_moves_end = 0;

void start_motion_unit(motion_direction_t _direction, uint8_t unit) {
  switch (_direction) {
    case FORWARD:
    case REVERSE:
      start_motion_distance(_direction, unit * kBlock_distance);
      break;
    case LEFT:
    case RIGHT:
      start_motion_distance(_direction, distanceToTicks(angleToDistance(unit * 45)));
      break;
  }
}

void start_motion_distance(motion_direction_t _direction, uint32_t distance) {
  if (num_moves >= kMovement_buffer_size) {
    Serial.println("movement buffer full");
    return;
  }

  buffered_moves[pos_moves_end].type = DISTANCE;
  buffered_moves[pos_moves_end].direction = _direction;
  buffered_moves[pos_moves_end].target = distance;

  num_moves ++;
  pos_moves_end ++;
  if (pos_moves_end >= kMovement_buffer_size) {
    pos_moves_end = 0;
  }
}

void start_motion_obstacle(uint16_t distance) {
  if (num_moves >= kMovement_buffer_size) {
    Serial.println("movement buffer full");
    return;
  }

  buffered_moves[pos_moves_end].type = OBSTACLE;
  buffered_moves[pos_moves_end].target = distance;

  num_moves ++;
  pos_moves_end ++;
  if (pos_moves_end >= kMovement_buffer_size) {
    pos_moves_end = 0;
  }
}

void parse_next_move() {
  static state_t pState = IDLE;

  if (pState != state) {
    if (state == IDLE) {
      Serial.println("move done");
    }
  }

  if (num_moves == 0 || state != IDLE) {
    return;
  }

  Serial.print("Parsing motion buffer index=");
  Serial.println(pos_moves_start);

  cli();
  axis_left.resetEncoder();
  axis_right.resetEncoder();
  state = MOVE_COMMANDED;

  // track how many moves have been parsed
  // this can be more than one when moves are combined, eg F1 + F1 = F2
  uint8_t parsed_moves = 1;

  switch (buffered_moves[pos_moves_start].type) {
    case DISTANCE:
      {
        move_type = DISTANCE;
        target = buffered_moves[pos_moves_start].target;
        motion_direction_t direction = buffered_moves[pos_moves_start].direction;

        // parse the next moves to see if we can combine them together
        for (uint8_t i = 1; i < num_moves; i++) {
          uint8_t move_index = (pos_moves_start + i) % kMovement_buffer_size;

          if (
            buffered_moves[move_index].type != DISTANCE ||
            buffered_moves[move_index].direction != direction
          ) {
            break;
          }

          Serial.print("Combining move index=");
          Serial.println(move_index);

          target += buffered_moves[move_index].target;
          parsed_moves ++;
        }

        switch (direction) {
          case FORWARD:
            axis_left.setReverse(false);
            axis_right.setReverse(false);
            Serial.print("start forward - target=");
            Serial.println(target);
            break;
          case REVERSE:
            axis_left.setReverse(true);
            axis_right.setReverse(true);
            Serial.print("start reverse - target=");
            Serial.println(target);
            break;
          case LEFT:
            axis_left.setReverse(true);
            axis_right.setReverse(false);
            Serial.print("start left turn - target=");
            Serial.println(target);
            break;
          case RIGHT:
            axis_left.setReverse(false);
            axis_right.setReverse(true);
            Serial.print("start right turn - target=");
            Serial.println(target);
            break;
        }
        break;
      }
    case OBSTACLE:
      move_type = OBSTACLE;
      target = buffered_moves[pos_moves_start].target;
      axis_left.setReverse(false);
      axis_right.setReverse(false);
      Serial.println("start move to obstacle");
      break;
  }
  sei();

  num_moves -= parsed_moves;
  pos_moves_start = (pos_moves_start + parsed_moves) % kMovement_buffer_size;
}

void start_align() {
  int16_t error_sensors = sensor_distances[LEFT_FRONT] - sensor_distances[LEFT_REAR];

  if (abs(error_sensors) >= align_LUT_len) {
    Serial.println("Offset too much to correct");
    return;
  }

  if (error_sensors > 0) {
    start_motion_distance(LEFT, pgm_read_byte_near(align_LUT + error_sensors));
  } else {
    start_motion_distance(RIGHT, pgm_read_byte_near(align_LUT - error_sensors));
  }
}

bool log_motion = false;
bool parse_moves = true;

void loop_motion() {
  if (parse_moves) {
    parse_next_move();
  }

  if (log_motion) {
    static uint32_t last_print = 0;
    uint32_t cur_time = millis();

    if ((cur_time - last_print) > 10) {
      cli();
      int32_t __encoder_left = axis_left.getEncoder();
      int32_t __encoder_right = axis_right.getEncoder();
      int16_t power_left = axis_left.getPower();
      int16_t power_right = axis_right.getPower();
      // int16_t _base_power = base_power;
      int16_t _correction = correction;
      int16_t _error = __encoder_left - __encoder_right;
      int16_t _sensor_front = sensor_distances[LEFT_FRONT];
      int16_t _sensor_rear = sensor_distances[LEFT_REAR];
      sei();
      Serial.print("SYNC");
      Serial.write((char *) &__encoder_left, 4);
      Serial.write((char *) &__encoder_right, 4);
      Serial.write((char *) &power_left, 2);
      Serial.write((char *) &power_right, 2);
      // Serial.write((char *) &_error, 2);
      // Serial.write((char *) &_correction, 2);
      Serial.write((char *) &_sensor_front, 2);
      Serial.write((char *) &_sensor_rear, 2);
      Serial.write((char *) &state, 1);
      Serial.println();

      last_print = cur_time;
    }
  }
}
