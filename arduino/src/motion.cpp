#include <Arduino.h>

#include "motion.h"
#include "motion_init.h"
#include "config.h"
#include "board.h"
#include "sensors.h"
#include "physical.h"
#include "align_LUT.h"

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
  uint8_t unit;
} move_t;

typedef struct {
  int16_t integral = 0;
  int32_t last_input = 0;
} pid_state_t;

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

move_t buffered_moves[kMovement_buffer_size];
uint8_t num_moves = 0;
uint8_t pos_moves_start = 0;
uint8_t pos_moves_end = 0;

static state_t state = IDLE;
static move_type_t move_type = DISTANCE;
static motion_direction_t move_dir;

static int32_t target_left = 0;
static int32_t target_right = 0;
static int16_t target_obstacle = 0;

volatile int16_t base_left = 0;
volatile int16_t base_right = 0;
volatile int16_t correction = 0;

// whether track left controller is enabled
static bool straight_enabled = true;

void combine_next_move();

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

  // whether the left motor has reached max power
  static bool reached_max_power = false;

  // whether the encoder has changed from the last update
  bool has_encoder_delta = (delta_left < -kEncoder_move_threshold) || (delta_left > kEncoder_move_threshold) ||
                  (delta_right < -kEncoder_move_threshold) || (delta_right > kEncoder_move_threshold);

  // distance travelled after which the robot will report it has moved a tile
  static uint16_t report_block_threshold;
  // distance travelled after which the robot will report sensor data
  static uint16_t report_sensor_threshold;
  static bool has_reported_sensors = true;

  if (state == MOVING && !has_encoder_delta) {
    // encoder delta has slowed to almost zero - lets check if we should finish the move
    int32_t diff_err = encoder_left - encoder_right;
    if (!straight_enabled || (diff_err > -kMax_encoder_diff_error && diff_err < kMax_encoder_diff_error)) {
      // if the error between both encoders are really similar, we can check for the
      // move-specific terminating condition
      if (move_type == DISTANCE) {
        // DISTANCE terminates when both encoders are close to target
        int32_t diff_left = encoder_left - target_left;
        int32_t diff_right = encoder_right - target_right;
        if (
          (diff_left > -kMax_encoder_error && diff_left < kMax_encoder_error) && 
          (diff_right > -kMax_encoder_error && diff_right < kMax_encoder_error)
        ) {
          Serial.println("move distance done");
          state = IDLE;
          axis_left.setPower(0);
          axis_right.setPower(0);
          return;
        }
      } else if (move_type == OBSTACLE) {
        // OBSTACLE terminates when the sensor distance is really close to target
        int16_t diff_err = sensor_distances[FRONT_FRONT_MID] - target_obstacle;
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
    report_block_threshold = 0;
    has_reported_sensors = true;
    reached_max_power = false;
    straight_enabled = true;
  }

  int16_t power_left = 0, power_right = 0;
  if (move_type == DISTANCE) {
    base_left = controllerStraight(&state_straight_left, encoder_left, target_left);
    base_right = controllerStraight(&state_straight_right, encoder_right, target_right);

    // crude way to determine if robot is decelerating:
    // for most of the move, controllerStraight will be saturated
    // when it starts to fall, we're nearing the end of the move
    if (base_left >= kMS_max_output) {
      reached_max_power = true;
    } else if (reached_max_power) {
      reached_max_power = false;
      Serial.println("decelerating");
      // combine_next_move();
    }
  } else if (move_type == OBSTACLE) {
    base_left = controllerObstacle(&state_obstacle, sensor_distances[FRONT_FRONT_MID], target_obstacle);
    base_right = base_left;
  }

  if (straight_enabled) {
    correction = controllerTrackLeft(encoder_left, encoder_right);
  } else {
    correction = 0;
  }

  power_left = base_left - correction;
  power_right = base_right + correction;

  axis_left.setPower(power_left);
  axis_right.setPower(power_right);

  // we treat report_block_threshold == 0 as a "initialise the value" condition
  // so that we don't have to duplicate the logic for incrementing dist_actual into the MOVE_COMMANDED handler
  if (encoder_left > report_block_threshold || report_block_threshold == 0) {
    uint16_t pReport_block_threshold = report_block_threshold;

    if (report_block_threshold > 0) {
      switch(move_dir) {
        case FORWARD:
          Serial.println("$UF");
          report_block_threshold += kBlock_distance;
          break;
        case REVERSE:
          Serial.println("$UB");
          report_block_threshold += kBlock_distance;
          break;
        case LEFT:
          Serial.println("$UL");
          report_block_threshold += kTicks_per_45_degrees * 2;
          break;
        case RIGHT:
          Serial.println("$UR");
          report_block_threshold += kTicks_per_45_degrees * 2;
          break;
      }
    } else {
      report_block_threshold = kReport_distance;
    }

    // if we will eventually travel fully past this block, delay logging the sensors
    // until we move past it by kReport_distance_offset to get a more accurate reading
    // use the previous threshold value if already initialised, else freshly initialised value
    uint16_t base_threshold = pReport_block_threshold > 0 ? pReport_block_threshold : report_block_threshold;
    uint16_t offset_target = base_threshold + kReport_distance_offset;

    if (offset_target < target_left) {
      report_sensor_threshold = offset_target;
    } else {
      report_sensor_threshold = base_threshold;
    }
    has_reported_sensors = false;
  }

  if (!has_reported_sensors && encoder_left > report_sensor_threshold) {
    log_all_sensors();
    has_reported_sensors = true;
  }
}

bool get_motion_done() {
  return state == IDLE;
}

int32_t get_encoder_left() {
  return axis_left.getEncoder();
}

void start_motion_unit(motion_direction_t _direction, uint8_t unit) {
  if (num_moves >= kMovement_buffer_size) {
    Serial.println("movement buffer full");
    return;
  }

  buffered_moves[pos_moves_end].type = DISTANCE;
  buffered_moves[pos_moves_end].direction = _direction;
  buffered_moves[pos_moves_end].unit = unit;

  if (_direction == FORWARD || _direction == REVERSE) {
    buffered_moves[pos_moves_end].target = unit * kBlock_distance;
  } else if (_direction == LEFT || _direction == RIGHT) {
    buffered_moves[pos_moves_end].target = unit * kTicks_per_45_degrees;
  }

  num_moves ++;
  pos_moves_end ++;
  if (pos_moves_end >= kMovement_buffer_size) {
    pos_moves_end = 0;
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
  buffered_moves[pos_moves_end].unit = 0;

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
        int32_t target = buffered_moves[pos_moves_start].target;
        move_dir = buffered_moves[pos_moves_start].direction;

        // parse the next moves to see if we can combine them together
        for (uint8_t i = 1; i < num_moves; i++) {
          uint8_t move_index = (pos_moves_start + i) % kMovement_buffer_size;

          if (
            buffered_moves[move_index].type != DISTANCE ||
            buffered_moves[move_index].direction != move_dir
          ) {
            break;
          }

          Serial.print("Combining move index=");
          Serial.println(move_index);

          target += buffered_moves[move_index].target;
          parsed_moves ++;
        }

        switch (move_dir) {
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
        target_left = target;
        target_right = target;
        break;
      }
    case OBSTACLE:
      move_type = OBSTACLE;
      target_obstacle = buffered_moves[pos_moves_start].target;
      axis_left.setReverse(false);
      axis_right.setReverse(false);
      Serial.println("start move to obstacle");
      break;
  }
  sei();

  num_moves -= parsed_moves;
  pos_moves_start = (pos_moves_start + parsed_moves) % kMovement_buffer_size;
}

void combine_next_move() {
  if (num_moves == 0) {
    return;
  }

  if (move_dir == FORWARD) {
    if (
      buffered_moves[pos_moves_start].type == DISTANCE &&
      (
        buffered_moves[pos_moves_start].direction == LEFT ||
        buffered_moves[pos_moves_start].direction == RIGHT
      )
    ) {
      if (buffered_moves[pos_moves_start].direction == LEFT) {
        target_right += buffered_moves[pos_moves_start].unit * kTicks_per_45_degrees_combined;
        Serial.println("Starting left turn early");
      } else if (buffered_moves[pos_moves_start].direction == RIGHT) {
        target_left += buffered_moves[pos_moves_start].unit * kTicks_per_45_degrees_combined;
        Serial.println("Starting right turn early");
      }

      // disable straight controller since we will now intentionally move the axis asymmetrically
      straight_enabled = false;

      num_moves --;
      pos_moves_start = (pos_moves_start + 1) % kMovement_buffer_size;
    }
  }
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
