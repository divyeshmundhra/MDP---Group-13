#include <Arduino.h>

#include "motion.h"
#include "motion_init.h"
#include "motion_profile.h"
#include "config.h"
#include "board.h"
#include "sensors.h"
#include "physical.h"
#include "align_LUT.h"

typedef enum {
  IDLE,
  MOVE_COMMANDED,
  MOVING,
  REPORT_SENSOR_INIT,
  REPORT_SENSOR
} state_t;

typedef enum {
  DISTANCE,
  OBSTACLE,
  ALIGN_EQUAL
} move_type_t;

typedef enum {
  ALIGN_IDLE,
  ALIGN_LEFT,
  ALIGN_LEFT_FRONT,
  ALIGN_RIGHT_FRONT,
  ALIGN_FORWARD
} align_type_t;

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

// stores flags for printing of a particular log statement
struct {
  uint16_t move_distance_done : 1;
  uint16_t move_obstacle_done : 1;
  uint16_t decelerating : 1;
  uint16_t update_f : 1;
  uint16_t update_b : 1;
  uint16_t update_l : 1;
  uint16_t update_r : 1;
  uint16_t emergency_stop : 1;
} display;

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
int16_t controllerMotionProfile(pid_state_t *state, int32_t encoder, setpoint_t *sp) {
  // controller aiming to make encoder track setpoint
  int32_t error = sp->pos - encoder;

  int16_t power = (
    (int32_t) kV_mp * sp->vel +
    (int32_t) kA_mp * sp->accel +
    (int32_t) kP_mp * error +
    (int32_t) kD_mp * (error - state->last_input)
  ) >> 8;

  state->last_input = error;

  if (power > kMP_max_output) {
    return kMP_max_output;
  } else if (power < kMP_min_output) {
    return kMP_min_output;
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

pid_state_t state_wall_align_equal;
int16_t controllerWallAlignEqual(pid_state_t *state, int16_t sensor_slave, int16_t sensor_target) {
  int16_t error = sensor_slave - sensor_target;

  state->integral = constrain((int32_t) state->integral + error, kA_integral_min, kA_integral_max);
  int16_t power = ((int32_t) kP_align * error + (int32_t) kI_align * state->integral + (int32_t) kD_align * (state->last_input - sensor_slave)) >> 8;

  state->last_input = sensor_slave;

  return constrain(power, kA_min_output, kA_max_output);
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

volatile uint16_t last_align_target = 0;

void combine_next_move();

volatile align_type_t align_type;
bool is_valid_align_target(align_type_t type) {
  switch (type) {
    case ALIGN_LEFT:
      {
        if (!sensor_stable[LEFT_FRONT] || !sensor_stable[LEFT_REAR]) {
          return false;
        }

        if (
            sensor_distances[LEFT_FRONT] > kWall_align_max_absolute_threshold ||
            sensor_distances[LEFT_REAR] > kWall_align_max_absolute_threshold
        ) {
          return false;
        }

        int16_t wall_diff = sensor_distances[LEFT_FRONT] - sensor_distances[LEFT_REAR];
        if (wall_diff <- kWall_align_max_absolute_difference || wall_diff > kWall_align_max_absolute_difference) {
          return false;
        }

        return true;
      }
    case ALIGN_LEFT_FRONT:
      if (!sensor_stable[LEFT_FRONT]) {
        return false;
      }

      if (sensor_distances[LEFT_FRONT] > kWall_align_max_absolute_threshold) {
        return false;
      }
      
      return true;
    case ALIGN_RIGHT_FRONT:
      if (!sensor_stable[RIGHT_FRONT]) {
        return false;
      }

      if (sensor_distances[RIGHT_FRONT] > kWall_align_max_absolute_threshold) {
        return false;
      }
      
      return true;
    case ALIGN_FORWARD:
      {
        if (!sensor_stable[FRONT_FRONT_LEFT] || !sensor_stable[FRONT_FRONT_RIGHT]) {
          return false;
        }

        if (
            sensor_distances[FRONT_FRONT_RIGHT] > kWall_align_max_absolute_threshold ||
            sensor_distances[FRONT_FRONT_LEFT] > kWall_align_max_absolute_threshold
        ) {
          return false;
        }

        int16_t wall_diff = sensor_distances[FRONT_FRONT_RIGHT] - sensor_distances[FRONT_FRONT_LEFT];
        if (wall_diff <- kWall_align_max_absolute_difference || wall_diff > kWall_align_max_absolute_difference) {
          return false;
        }

        return true;
      }
    default:
      return false;
  }
}

volatile int16_t align_target_offset = 0;

uint8_t get_base_wall_align_offset(align_type_t align_type, int16_t val) {
  switch (align_type) {
    case ALIGN_LEFT:
    case ALIGN_LEFT_FRONT:
      if (val > 400) {
        return val;
      }

      return kWall_offsets_left[val / 100];
      break;
    case ALIGN_RIGHT_FRONT:
      if (val > 400) {
        return val;
      }

      return kWall_offsets_right[val / 100];
      break;
    default:
      break;
  }

  return val;
}

Motion_Profile mp;

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  static int32_t pEncoder_left = 0;
  static int32_t pEncoder_right = 0;

  if (state == IDLE) {
    axis_left.setBrake(400);
    axis_right.setBrake(400);
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

  static bool has_ebraked = false;
  if (!has_ebraked && state == MOVING && move_type == DISTANCE && move_dir == FORWARD) {
    if (sensor_distances[FRONT_FRONT_MID] < kEmergency_brake_threshold) {
      display.emergency_stop = 1;
      target_left = target_right = encoder_left + kEmergency_brake_correction;
      has_ebraked = true;
    }
  }

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
          display.move_distance_done = 1;

          switch(move_dir) {
            case FORWARD:
              display.update_f = 1;
              break;
            case REVERSE:
              display.update_b = 1;
              break;
            case LEFT:
              display.update_l = 1;
              break;
            case RIGHT:
              display.update_r = 1;
              break;
          }

          state = REPORT_SENSOR_INIT;
          axis_left.setBrake(400);
          axis_right.setBrake(400);
          return;
        }
      } else if (move_type == OBSTACLE) {
        // OBSTACLE terminates when the sensor distance is really close to target
        int16_t diff_err = sensor_distances[FRONT_FRONT_MID] - target_obstacle;
        if (diff_err > -kMax_obstacle_error && diff_err < kMax_obstacle_error) {
          display.move_obstacle_done = 1;
          state = IDLE;
          axis_left.setBrake(400);
          axis_right.setBrake(400);
          return;
        }
      }
    }
  } else if (state == MOVE_COMMANDED) {
    if (move_type == ALIGN_EQUAL) {
      state = MOVING;
      straight_enabled = false;
      resetControllerState(&state_wall_align_equal, sensor_distances[LEFT_FRONT]);
    } else {
      resetControllerState(&state_tl, encoder_right);
      if (move_type == DISTANCE) {
        resetControllerState(&state_straight_left, encoder_left);
        resetControllerState(&state_straight_right, encoder_right);
        has_ebraked = false;

        mp.init(target_left);
      } else if (move_type == OBSTACLE) {
        resetControllerState(&state_obstacle, sensor_distances[FRONT_FRONT_LEFT]);
      }
      state = MOVING;
    }
  }

  int16_t power_left = 0, power_right = 0;
  if (move_type == DISTANCE) {
    setpoint_t sp;

    if (!mp.step(&sp)) {
      display.move_distance_done = 1;

      switch(move_dir) {
        case FORWARD:
          display.update_f = 1;
          break;
        case REVERSE:
          display.update_b = 1;
          break;
        case LEFT:
          display.update_l = 1;
          break;
        case RIGHT:
          display.update_r = 1;
          break;
      }

      state = REPORT_SENSOR_INIT;
      axis_left.setBrake(400);
      axis_right.setBrake(400);
      return;
    }
    base_left = controllerMotionProfile(&state_straight_left, encoder_left, &sp);
    base_right = controllerMotionProfile(&state_straight_right, encoder_right, &sp);
  } else if (move_type == OBSTACLE) {
    base_left = controllerObstacle(&state_obstacle, sensor_distances[FRONT_FRONT_LEFT], target_obstacle);
    base_right = base_left;
  } else if (move_type == ALIGN_EQUAL) {
    static uint8_t tick_count = 0;
    tick_count ++;

    if (tick_count > 4) {
      tick_count = 0;

      int16_t power = controllerWallAlignEqual(&state_wall_align_equal, sensor_distances[LEFT_FRONT], sensor_distances[LEFT_REAR]);
      base_left = -power;
      base_right = power;
    }
  }

  if (false && straight_enabled) {
    static uint8_t tick_count = 0;
    tick_count ++;

    if (tick_count > 4 && (base_left > kWall_align_min_power) && (base_right > kWall_align_min_power)) {
      tick_count = 0;

      if (move_dir == FORWARD) {
        static uint8_t ticks_since_align_selected = 0;
        ticks_since_align_selected ++;

        if (align_type == ALIGN_IDLE && ticks_since_align_selected >= 5) {
          ticks_since_align_selected = 0;

          if (move_type != OBSTACLE) {
            int16_t base_offset = 0;
            int16_t diff = 0;
            int16_t sensor_val = 0;

            if (is_valid_align_target(ALIGN_LEFT)) {
              align_type = ALIGN_LEFT;
              sensor_val = sensor_distances[LEFT_FRONT] % 100;
              base_offset = get_base_wall_align_offset(ALIGN_LEFT, sensor_distances[LEFT_FRONT]);
            } else if (is_valid_align_target(ALIGN_LEFT_FRONT)) {
              align_type = ALIGN_LEFT_FRONT;
              sensor_val = sensor_distances[LEFT_FRONT] % 100;
              base_offset = get_base_wall_align_offset(ALIGN_LEFT_FRONT, sensor_distances[LEFT_FRONT]);
            } else if (is_valid_align_target(ALIGN_RIGHT_FRONT)) {
              align_type = ALIGN_RIGHT_FRONT;
              sensor_val = sensor_distances[RIGHT_FRONT] % 100;
              base_offset = get_base_wall_align_offset(ALIGN_RIGHT_FRONT, sensor_distances[RIGHT_FRONT]);
            }

            // compute diff between current sensor reading and the ideal offset
            // if too far, use the current sensor reading as the offset target instead
            diff = sensor_val - base_offset;
            if (diff > -kWall_align_max_offset_delta && diff < kWall_align_max_offset_delta) {
              align_target_offset = base_offset;
            } else {
              // align_target_offset = sensor_val;
              align_type = ALIGN_IDLE;
            }
          } else {
            // obstacle will only align forwards
            if (is_valid_align_target(ALIGN_FORWARD)) {
              align_type = ALIGN_FORWARD;
            }
          }
        } else {
          if (!is_valid_align_target(align_type)) {
            align_type = ALIGN_IDLE;
          }
        }

        switch (align_type) {
          case ALIGN_LEFT: {
            int16_t sensor_left_front_block = sensor_distances[LEFT_FRONT] % 100;
            int16_t sensor_left_rear_block = sensor_distances[LEFT_REAR] % 100;

            int16_t wall_diff = sensor_left_front_block - sensor_left_rear_block;
            int16_t wall_offset = sensor_left_front_block - align_target_offset;

            int32_t wall_correction = ((int32_t) kP_wall_diff_left * wall_diff + (int32_t) kP_wall_offset_left * wall_offset) >> 8;

            if (wall_correction > 0) {
              axis_right.incrementEncoder(-wall_correction);
            } else {
              axis_left.incrementEncoder(wall_correction);
            }

            break;
          }
          case ALIGN_FORWARD:
          {
            int16_t wall_diff = sensor_distances[FRONT_FRONT_RIGHT] - sensor_distances[FRONT_FRONT_LEFT];

            int32_t wall_correction = ((int32_t) kP_wall_diff_forward * wall_diff) >> 8;

            if (wall_correction > 0) {
              axis_right.incrementEncoder(-wall_correction);
            } else {
              axis_left.incrementEncoder(wall_correction);
            }
            
            break;
          }
          case ALIGN_LEFT_FRONT:
          {
            int16_t sensor_left_front_block = sensor_distances[LEFT_FRONT] % 100;
            int16_t wall_offset = sensor_left_front_block - align_target_offset;

            int32_t wall_correction = ((int32_t) kP_wall_offset_left * wall_offset) >> 8;

            if (wall_correction > 0) {
              axis_right.incrementEncoder(-wall_correction);
            } else {
              axis_left.incrementEncoder(wall_correction);
            }

            break;
          }
          case ALIGN_RIGHT_FRONT:
          {
            int16_t sensor_right_front_block = sensor_distances[RIGHT_FRONT] % 100;
            int16_t wall_offset = sensor_right_front_block - align_target_offset;

            int32_t wall_correction = ((int32_t) kP_wall_offset_right * wall_offset) >> 8;

            if (wall_correction > 0) {
              axis_left.incrementEncoder(-wall_correction);
            } else {
              axis_right.incrementEncoder(wall_correction);
            }

            break;
          }
        }
      }
    }
    correction = controllerTrackLeft(encoder_left, encoder_right);
  } else {
    correction = 0;
  }

  power_left = base_left - correction;
  power_right = base_right + correction;

  axis_left.setPower(power_left);
  axis_right.setPower(power_right);
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

  if (num_moves == 0 && (state == MOVE_COMMANDED || state == MOVING) && move_type == DISTANCE && move_dir == _direction) {
    int32_t new_move = 0;
    if (_direction == FORWARD || _direction == REVERSE) {
      new_move = unit * kBlock_distance;
    } else if (_direction == LEFT || _direction == RIGHT) {
      new_move = unit_turn_to_ticks(unit);
    }

    target_left += new_move;
    target_right += new_move;
    Serial.println("Combined move with active");
    return;
  }

  // attempt to combine moves together if they are of the same type and direction
  if (num_moves > 0) {
    uint8_t prev_move = pos_moves_end == 0 ? (kMovement_buffer_size - 1) : pos_moves_end - 1;

    if (
      buffered_moves[prev_move].type == DISTANCE &&
      buffered_moves[prev_move].direction == _direction
    ) {
      buffered_moves[prev_move].unit += unit;

      if (_direction == FORWARD || _direction == REVERSE) {
        buffered_moves[prev_move].target = buffered_moves[prev_move].unit * kBlock_distance;
      } else if (_direction == LEFT || _direction == RIGHT) {
        buffered_moves[prev_move].target = unit_turn_to_ticks(buffered_moves[prev_move].unit);
      }

      Serial.print("Combined move with index=");
      Serial.println(prev_move);
      return;
    }
  }

  buffered_moves[pos_moves_end].type = DISTANCE;
  buffered_moves[pos_moves_end].direction = _direction;
  buffered_moves[pos_moves_end].unit = unit;

  if (_direction == FORWARD || _direction == REVERSE) {
    buffered_moves[pos_moves_end].target = unit * kBlock_distance;
  } else if (_direction == LEFT || _direction == RIGHT) {
    buffered_moves[pos_moves_end].target = unit_turn_to_ticks(unit);
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
  if (num_moves == 0 || state != IDLE) {
    return;
  }

  Serial.print("Parsing motion buffer index=");
  Serial.println(pos_moves_start);

  cli();
  axis_left.resetEncoder();
  axis_right.resetEncoder();
  state = MOVE_COMMANDED;

  switch (buffered_moves[pos_moves_start].type) {
    case DISTANCE:
      {
        move_type = DISTANCE;
        int32_t target = buffered_moves[pos_moves_start].target;
        move_dir = buffered_moves[pos_moves_start].direction;

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
      move_dir = FORWARD;
      target_obstacle = buffered_moves[pos_moves_start].target;
      axis_left.setReverse(false);
      axis_right.setReverse(false);
      Serial.println("start move to obstacle");
      break;
  }
  sei();

  num_moves -= 1;
  pos_moves_start = (pos_moves_start + 1) % kMovement_buffer_size;
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

void start_align(uint8_t mode) {
  if (mode == 0) {
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
  } else if (mode == 1) {
    cli();
    state = MOVE_COMMANDED;
    move_type = ALIGN_EQUAL;
    axis_left.setReverse(false);
    axis_right.setReverse(false);
    sei();

    Serial.println("Start align equal");
  }
}

bool log_motion = false;
bool parse_moves = true;

void loop_motion() {
  static align_type_t pAlign_type = ALIGN_IDLE;
  static uint32_t report_delay_start = 0;

  if (state == REPORT_SENSOR_INIT) {
    report_delay_start = millis();
    state = REPORT_SENSOR;
  }

  if (pAlign_type != align_type) {
    Serial.print("Align change: ");
    Serial.println(align_type);

    pAlign_type = align_type;
  }

  if (parse_moves) {
    parse_next_move();
  }

  if (log_motion) {
    static uint32_t last_print = 0;
    uint32_t cur_time = millis();

    if ((cur_time - last_print) > 10) {
      cli();
      int32_t encoder_left = axis_left.getEncoder();
      int32_t encoder_right = axis_right.getEncoder();
      int16_t _base_left = base_left;
      int16_t _base_right = base_right;
      int16_t _sensor_front = sensor_distances[LEFT_FRONT];
      int16_t _sensor_rear = sensor_distances[LEFT_REAR];
      sei();
      Serial.print("SYNC");
      Serial.write((char *) &encoder_left, 4);
      Serial.write((char *) &encoder_right, 4);
      Serial.write((char *) &_base_left, 2);
      Serial.write((char *) &_base_right, 2);
      // Serial.write((char *) &_error, 2);
      // Serial.write((char *) &_correction, 2);
      Serial.write((char *) &_sensor_front, 2);
      Serial.write((char *) &_sensor_rear, 2);
      Serial.write((char *) &state, 1);
      Serial.write((char *) &align_type, 1);
      Serial.println();

      last_print = cur_time;
    }
  }

  if (display.emergency_stop) {
    Serial.print(F("emergency stop. travelled: "));
    Serial.println(axis_left.getEncoder());
    display.emergency_stop = 0;
  }
  if (display.move_distance_done) {
    Serial.println(F("move distance done"));
    display.move_distance_done = 0;
  }
  if (display.move_obstacle_done) {
    Serial.println(F("move obstacle done"));
    display.move_obstacle_done = 0;
  }
  if (display.decelerating) {
    Serial.println(F("decelerating"));
    display.decelerating = 0;
  }
  if (display.update_f) {
    Serial.println(F("$UF"));
    display.update_f = 0;
  }
  if (display.update_b) {
    Serial.println(F("$UB"));
    display.update_b = 0;
  }
  if (display.update_l) {
    Serial.println(F("$UL"));
    display.update_l = 0;
  }
  if (display.update_r) {
    Serial.println(F("$UR"));
    display.update_r = 0;
  }

  if (state == REPORT_SENSOR && (millis() - report_delay_start) > kSensor_report_delay) {
    log_all_sensors();
    state = IDLE;
  }
}
