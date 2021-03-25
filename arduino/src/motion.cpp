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
  MOVING,
  REPORT_SENSOR_INIT,
  REPORT_SENSOR,
  ALIGN_DELAY_INIT,
  ALIGN_DELAY
} state_t;

typedef enum {
  DISTANCE,
  OBSTACLE,
  ALIGN_EQUAL_LEFT,
  ALIGN_EQUAL_FORWARD,
} move_type_t;

typedef enum {
  ALIGN_IDLE,
  ALIGN_LEFT,
  ALIGN_FORWARD
} align_type_t;

typedef enum {
  ALIGN_AUTO_IDLE,
  ALIGN_AUTO_STATIC,
  ALIGN_AUTO_DYNAMIC,
  ALIGN_AUTO_TURN,
  ALIGN_AUTO_OBSTACLE,
  ALIGN_AUTO_UNTURN,
  ALIGN_AUTO_DONE,
} align_auto_state_t;

typedef struct {
  move_type_t type;
  motion_direction_t direction;
  int32_t target;
  uint8_t unit;
  bool align;
  bool report;
} move_t;

typedef struct {
  int16_t integral = 0;
  int32_t last_input = 0;
} pid_state_t;

// stores flags for printing of a particular log statement
struct {
  uint16_t move_distance_done : 1;
  uint16_t move_obstacle_done : 1;
  uint16_t align_done : 1;
  uint16_t decelerating : 1;
  uint16_t update_f : 1;
  uint16_t update_b : 1;
  uint16_t update_l : 1;
  uint16_t update_r : 1;
  uint16_t emergency_stop : 1;
  uint16_t force_end : 1;
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
int16_t controllerStraight(pid_state_t *state, int32_t encoder, int32_t target) {
  // controller aiming to make encoder track target
  int32_t error = target - encoder;

  state->integral = constrain((int32_t) state->integral + error, kMS_integral_min, kMS_integral_max);
  int16_t power = ((int32_t) kP_straight * error + (int32_t) kI_straight * state->integral + (int32_t) kD_straight * (state->last_input - encoder)) >> 8;

  state->last_input = encoder;

  if (power > kMS_max_output) {
    return kMS_max_output;
  } else if (power < kMS_min_output) {
    return kMS_min_output;
  }

  return power;
}

pid_state_t state_obstacle_left;
pid_state_t state_obstacle_right;
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
// whether alignment should be performed after this move
static bool move_align;
// whether the completion of this move should be reported
static bool move_report;

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
      if (val > 400) {
        return val;
      }

      return kWall_offsets_left[val / 100];
      break;
    default:
      break;
  }

  return val;
}

// triggers at 100Hz
ISR(TIMER2_COMPA_vect) {
  // reset timer counter
  TCNT2 = 0;

  static int32_t pEncoder_left = 0;
  static int32_t pEncoder_right = 0;

  if (state != MOVE_COMMANDED && state != MOVING) {
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

  static uint32_t zero_movement_since = 0;

  // whether we have seen zero movement for too long and should force end the move
  bool force_end = false;
  if (delta_left == 0 && delta_right == 0) {
    if (zero_movement_since == 0) {
      zero_movement_since = millis();
    }

    if ((millis() - zero_movement_since) > kZero_movement_timeout) {
      force_end = true;
      display.force_end = 1;
      zero_movement_since = 0;
    }
  } else {
    zero_movement_since = 0;
  }

  if (state == MOVING && (!has_encoder_delta || force_end)) {
    // encoder delta has slowed to almost zero - lets check if we should finish the move
    int32_t diff_err = encoder_left - encoder_right;
    if (force_end || !straight_enabled || (diff_err > -kMax_encoder_diff_error && diff_err < kMax_encoder_diff_error)) {
      // if the error between both encoders are really similar, we can check for the
      // move-specific terminating condition
      if (move_type == DISTANCE) {
        // DISTANCE terminates when both encoders are close to target
        int32_t diff_left = encoder_left - target_left;
        int32_t diff_right = encoder_right - target_right;
        if (
          (
            (diff_left > -kMax_encoder_error && diff_left < kMax_encoder_error) && 
            (diff_right > -kMax_encoder_error && diff_right < kMax_encoder_error)
          ) || force_end
        ) {
          display.move_distance_done = 1;

          if (move_report) {
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
          }

          state = REPORT_SENSOR_INIT;
          axis_left.resetEncoderForNextMove(target_left - axis_left.getEncoder());
          axis_right.resetEncoderForNextMove(target_right - axis_right.getEncoder());
          axis_left.setBrake(400);
          axis_right.setBrake(400);
          return;
        }
      } else if (move_type == OBSTACLE) {
        // OBSTACLE terminates when the sensor distance is really close to target
        int16_t diff_err_left = sensor_distances[FRONT_FRONT_LEFT] - target_obstacle;
        int16_t diff_err_right = sensor_distances[FRONT_FRONT_RIGHT] - target_obstacle;

        if (
          (
            diff_err_left > -kMax_obstacle_error && diff_err_left < kMax_obstacle_error &&
            diff_err_right > -kMax_obstacle_error && diff_err_right < kMax_obstacle_error
          ) || force_end
        ) {
          display.move_obstacle_done = 1;
          state = IDLE;
          axis_left.resetEncoder();
          axis_right.resetEncoder();
          axis_left.setBrake(400);
          axis_right.setBrake(400);
          return;
        }
      } else if (move_type == ALIGN_EQUAL_LEFT || move_type == ALIGN_EQUAL_FORWARD) {
        int16_t diff_err;

        if (move_type == ALIGN_EQUAL_LEFT) {
          diff_err = sensor_distances[LEFT_FRONT] - sensor_distances[LEFT_REAR];
        } else if (move_type == ALIGN_EQUAL_FORWARD) {
          diff_err = sensor_distances[FRONT_FRONT_RIGHT] - sensor_distances[FRONT_FRONT_LEFT];
        }

        if ((diff_err > -kMax_align_error && diff_err < kMax_align_error) || force_end) {
          display.align_done = 1;
          state = ALIGN_DELAY_INIT;
          axis_left.resetEncoder();
          axis_right.resetEncoder();
          axis_left.setBrake(400);
          axis_right.setBrake(400);
          return;
        }
      }
    }
  } else if (state == MOVE_COMMANDED) {
    if (move_type == ALIGN_EQUAL_LEFT || move_type == ALIGN_EQUAL_FORWARD) {
      state = MOVING;
      straight_enabled = false;

      int16_t init_val;
      if (move_type == ALIGN_EQUAL_LEFT) {
        init_val = sensor_distances[LEFT_FRONT];
      } else if (move_type == ALIGN_EQUAL_FORWARD) {
        init_val = sensor_distances[FRONT_FRONT_RIGHT];
      }

      resetControllerState(&state_wall_align_equal, init_val);
    } else {
      resetControllerState(&state_tl, encoder_right);
      if (move_type == DISTANCE) {
        resetControllerState(&state_straight_left, encoder_left);
        resetControllerState(&state_straight_right, encoder_right);
        has_ebraked = false;

        straight_enabled = true;
      } else if (move_type == OBSTACLE) {
        resetControllerState(&state_obstacle_left, sensor_distances[FRONT_FRONT_LEFT]);
        resetControllerState(&state_obstacle_right, sensor_distances[FRONT_FRONT_RIGHT]);
        straight_enabled = false;
      }
      state = MOVING;
    }
  }

  int16_t power_left = 0, power_right = 0;
  if (move_type == DISTANCE) {
    base_left = controllerStraight(&state_straight_left, encoder_left, target_left);
    base_right = controllerStraight(&state_straight_right, encoder_right, target_right);
  } else if (move_type == OBSTACLE) {
    base_left = controllerObstacle(&state_obstacle_left, sensor_distances[FRONT_FRONT_LEFT], target_obstacle);
    base_right = controllerObstacle(&state_obstacle_right, sensor_distances[FRONT_FRONT_RIGHT], target_obstacle);
  } else if (move_type == ALIGN_EQUAL_LEFT || move_type == ALIGN_EQUAL_FORWARD) {
    int16_t val_slave, val_target;
    
    if (move_type == ALIGN_EQUAL_LEFT) {
      val_slave = sensor_distances[LEFT_FRONT];
      val_target = sensor_distances[LEFT_REAR];
    } else if (move_type == ALIGN_EQUAL_FORWARD) {
      val_slave = sensor_distances[FRONT_FRONT_RIGHT];
      val_target = sensor_distances[FRONT_FRONT_LEFT];
    }

    int16_t power = controllerWallAlignEqual(&state_wall_align_equal, val_slave, val_target);
    base_left = -power;
    base_right = power;
  }

  #define DO_LIVE_ALIGNMENT
  #ifdef DO_LIVE_ALIGNMENT
    if (straight_enabled && move_type == DISTANCE) {
      if ((base_left > kWall_align_min_power) && (base_right > kWall_align_min_power)) {
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
              }/* else if (is_valid_align_target(ALIGN_FORWARD)) {
                align_type = ALIGN_FORWARD;
                sensor_val = sensor_distances[FRONT_FRONT_RIGHT] % 100;
                base_offset = get_base_wall_align_offset(ALIGN_FORWARD, sensor_distances[FRONT_FRONT_RIGHT]);
              }*/

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
            case ALIGN_IDLE:
              break;
          }
        }
      }
      correction = controllerTrackLeft(encoder_left, encoder_right);
    } else {
      correction = 0;
    }
  #else
    if (straight_enabled) {
      correction = controllerTrackLeft(encoder_left, encoder_right);
    } else {
      correction = 0;
    }
  #endif

  power_left = base_left - correction;
  power_right = base_right + correction;

  axis_left.setPower(power_left, true);
  axis_right.setPower(power_right, true);
}

bool get_motion_done() {
  return state == IDLE;
}

int32_t get_encoder_left() {
  return axis_left.getEncoder();
}

void start_motion_unit(motion_direction_t _direction, uint8_t unit, bool align, bool report) {
  if (num_moves >= kMovement_buffer_size) {
    Serial.println("movement buffer full");
    return;
  }

  // if (num_moves == 0 && (state == MOVE_COMMANDED || state == MOVING) && move_type == DISTANCE && move_dir == _direction) {
  //   int32_t new_move = 0;
  //   if (_direction == FORWARD || _direction == REVERSE) {
  //     new_move = unit * kBlock_distance;
  //   } else if (_direction == LEFT || _direction == RIGHT) {
  //     new_move = unit_turn_to_ticks(unit);
  //   }

  //   target_left += new_move;
  //   target_right += new_move;
  //   Serial.println("Combined move with active");
  //   return;
  // }

  // // attempt to combine moves together if they are of the same type and direction
  // if (num_moves > 0) {
  //   uint8_t prev_move = pos_moves_end == 0 ? (kMovement_buffer_size - 1) : pos_moves_end - 1;

  //   if (
  //     buffered_moves[prev_move].type == DISTANCE &&
  //     buffered_moves[prev_move].direction == _direction
  //   ) {
  //     buffered_moves[prev_move].unit += unit;

  //     if (_direction == FORWARD || _direction == REVERSE) {
  //       buffered_moves[prev_move].target = buffered_moves[prev_move].unit * kBlock_distance;
  //     } else if (_direction == LEFT || _direction == RIGHT) {
  //       buffered_moves[prev_move].target = unit_turn_to_ticks(buffered_moves[prev_move].unit);
  //     }

  //     Serial.print("Combined move with index=");
  //     Serial.println(prev_move);
  //     return;
  //   }
  // }

  buffered_moves[pos_moves_end].type = DISTANCE;
  buffered_moves[pos_moves_end].direction = _direction;
  buffered_moves[pos_moves_end].unit = unit;
  buffered_moves[pos_moves_end].align = align;
  buffered_moves[pos_moves_end].report = report;

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

void start_motion_distance(motion_direction_t _direction, uint32_t distance, bool align, bool report) {
  if (num_moves >= kMovement_buffer_size) {
    Serial.println("movement buffer full");
    return;
  }

  buffered_moves[pos_moves_end].type = DISTANCE;
  buffered_moves[pos_moves_end].direction = _direction;
  buffered_moves[pos_moves_end].target = distance;
  buffered_moves[pos_moves_end].unit = 0;
  buffered_moves[pos_moves_end].align = align;
  buffered_moves[pos_moves_end].report = report;

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

// compute the offset of the robot from obstacles in front using a particular sensor
int16_t get_forward_offset(sensor_position_t sensor, uint8_t data_index) {
  // data_index: index to lookup in constants for this sensors operation
  uint8_t blocks_away = sensor_distances[sensor] / 100;

  if (blocks_away >= kForward_align_count) {
    return 0;
  }

  int8_t delta = sensor_distances[sensor] - kForward_align_target[data_index][blocks_away];
  if (
    (delta <= -kForward_align_max_error || delta > kForward_align_max_error) &&
    sensor_distances[sensor] > kForward_align_always_align_threshold
  ) {
    return 0;
  }

  return ((int32_t) delta * kBlock_distance) / 100;
}

void parse_next_move() {
  if (num_moves == 0 || state != IDLE) {
    return;
  }

  Serial.print("Parsing motion buffer index=");
  Serial.println(pos_moves_start);

  cli();
  // Serial.print("Start move with left=");
  // Serial.print(axis_left.getEncoder());
  // Serial.print(" right=");
  // Serial.println(axis_right.getEncoder());
  state = MOVE_COMMANDED;

  if (buffered_moves[pos_moves_start].type == DISTANCE) {
    move_type = buffered_moves[pos_moves_start].type;
    move_dir = buffered_moves[pos_moves_start].direction;
    move_align = buffered_moves[pos_moves_start].align;
    move_report = buffered_moves[pos_moves_start].report;

    int32_t target = 0;

    if (move_type == DISTANCE && move_dir == FORWARD) {
      // cycle through front sensors to compute offset
      int16_t offset_move_distance = get_forward_offset(FRONT_FRONT_MID, 0);

      if (offset_move_distance != 0) {
        Serial.print(F("Correct with FRONT_MID: "));
        Serial.println(offset_move_distance);
      }

      if (offset_move_distance == 0) {
        offset_move_distance = get_forward_offset(FRONT_FRONT_LEFT, 1);
        if (offset_move_distance != 0) {
          Serial.print(F("Correct with FRONT_LEFT: "));
          Serial.println(offset_move_distance);
        }
      }

      if (offset_move_distance == 0) {
        offset_move_distance = get_forward_offset(FRONT_FRONT_RIGHT, 2);
        if (offset_move_distance != 0) {
          Serial.print(F("Correct with FRONT_RIGHT: "));
          Serial.println(offset_move_distance);
        }
      }

      target = buffered_moves[pos_moves_start].target + offset_move_distance;
    } else {
      target = buffered_moves[pos_moves_start].target;
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

    Serial.print("move_align: ");
    Serial.println(move_align);
    target_left = target;
    target_right = target;
  } else if (buffered_moves[pos_moves_start].type == OBSTACLE) {
    move_type = OBSTACLE;
    move_dir = FORWARD;
    target_obstacle = buffered_moves[pos_moves_start].target;
    axis_left.setReverse(false);
    axis_right.setReverse(false);
    Serial.println("start move to obstacle");
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

    state = MOVE_COMMANDED;
    move_type = DISTANCE;
    if (error_sensors > 0) {
      move_dir = LEFT;
      move_align = false;
      move_report = false;
      axis_left.setReverse(true);
      axis_right.setReverse(false);
      target_left = target_right = pgm_read_word_near(align_left_LUT + error_sensors);
    } else {
      move_dir = LEFT;
      move_align = false;
      move_report = false;
      axis_left.setReverse(false);
      axis_right.setReverse(true);
      target_left = target_right = pgm_read_word_near(align_left_LUT - error_sensors);
    }
  } else if (mode == 1) {
    int16_t error_sensors = sensor_distances[FRONT_FRONT_RIGHT] - sensor_distances[FRONT_FRONT_LEFT];

    if (abs(error_sensors) >= align_LUT_len) {
      Serial.println("Offset too much to correct");
      return;
    }

    state = MOVE_COMMANDED;
    move_type = DISTANCE;
    if (error_sensors > 0) {
      move_dir = LEFT;
      move_align = false;
      move_report = false;
      axis_left.setReverse(true);
      axis_right.setReverse(false);
      target_left = target_right = pgm_read_word_near(align_forward_LUT + error_sensors);
    } else {
      move_dir = LEFT;
      move_align = false;
      move_report = false;
      axis_left.setReverse(false);
      axis_right.setReverse(true);
      target_left = target_right = pgm_read_word_near(align_forward_LUT - error_sensors);
    }
  } else if (mode == 2 || mode == 3) {
    cli();
    state = MOVE_COMMANDED;
    if (mode == 2) {
      move_type = ALIGN_EQUAL_LEFT;
    } else if (mode == 3) {
      move_type = ALIGN_EQUAL_FORWARD;
    }
    move_align = false;
    move_report = false;
    axis_left.setReverse(false);
    axis_right.setReverse(false);
    sei();

    Serial.print("Start align equal: ");
    Serial.println(mode);
  }
}

bool log_motion = false;
bool parse_moves = true;

void loop_motion() {
  static align_type_t pAlign_type = ALIGN_IDLE;
  static uint32_t report_delay_start = 0;
  static uint32_t align_delay = 0;
  static uint32_t time_since_idle = 0;

  static state_t pState = IDLE;

  if (pState != state) {
    if (state == IDLE) {
      time_since_idle = millis();
    }

    pState = state;
  }

  static align_auto_state_t align_auto_state = ALIGN_AUTO_IDLE;
  // direction when the alignment was started
  static motion_direction_t align_start_move_dir;
  static motion_direction_t align_dir;

  static uint8_t moves_since_turn_align = 0;
  static uint8_t solid_on_right = 0;
  static uint8_t solid_right_distance = 0;

  if (state == REPORT_SENSOR_INIT) {
    report_delay_start = millis();
    state = REPORT_SENSOR;
  } else if (state == ALIGN_DELAY_INIT) {
    align_delay = millis();
    state = ALIGN_DELAY;
  } else if (state == ALIGN_DELAY && (millis() - align_delay) > kAlign_delay) {
    state = IDLE;
    Serial.println(F("Align delay done"));

    if (align_auto_state == ALIGN_AUTO_DYNAMIC) {
      Serial.println(F("Finished dynamic alignment, start turn"));
      if (align_start_move_dir == FORWARD) {
        align_auto_state = ALIGN_AUTO_TURN;
      } else {
        align_auto_state = ALIGN_AUTO_IDLE;
      }
    }
  } else if (state == IDLE && move_type == OBSTACLE) {
    if (align_auto_state == ALIGN_AUTO_OBSTACLE) {
      Serial.println(F("Finished obstacle align, start unturn"));
      align_auto_state = ALIGN_AUTO_UNTURN;
    }
  }

  if (pAlign_type != align_type) {
    Serial.print("Align change: ");
    Serial.println(align_type);

    pAlign_type = align_type;
  }

  if (state == IDLE && parse_moves && (millis() - time_since_idle) > kAlign_delay) {
    switch (align_auto_state) {
      case ALIGN_AUTO_IDLE:
        parse_next_move();
        break;
      case ALIGN_AUTO_STATIC:
        if (state == IDLE) {
          if (
            sensor_distances[FRONT_FRONT_LEFT] < kAuto_align_threshold &&
            sensor_distances[FRONT_FRONT_RIGHT] < kAuto_align_threshold
          ) {
            int16_t diff_err = sensor_distances[FRONT_FRONT_LEFT] - sensor_distances[FRONT_FRONT_RIGHT];

            if (
              (diff_err > -kAuto_align_max_diff && diff_err < kAuto_align_max_diff) &&
              (diff_err <= -kAuto_align_min_diff || diff_err >= kAuto_align_min_diff)
            ) {
              start_align(1);
            }
          } else if (
            sensor_distances[LEFT_FRONT] < kAuto_align_threshold &&
            sensor_distances[LEFT_REAR] < kAuto_align_threshold
          ) {
            int16_t diff_err = sensor_distances[LEFT_FRONT] - sensor_distances[LEFT_REAR];

            if (
              (diff_err > -kAuto_align_max_diff && diff_err < kAuto_align_max_diff) &&
              (diff_err <= -kAuto_align_min_diff || diff_err >= kAuto_align_min_diff)
            ) {
              start_align(0);
            }
          }

          // no alignment was started, move on to offset alignment
          if (state == IDLE) {
            Serial.println(F("Did not start static alignment, moving to try forward alignment"));
            align_auto_state = ALIGN_AUTO_TURN;
          }
        }
        break;
      case ALIGN_AUTO_DYNAMIC:
        if (state == IDLE) {
          if (
            sensor_distances[FRONT_FRONT_LEFT] < kAuto_align_threshold &&
            sensor_distances[FRONT_FRONT_RIGHT] < kAuto_align_threshold
          ) {
            int16_t diff_err = sensor_distances[FRONT_FRONT_LEFT] - sensor_distances[FRONT_FRONT_RIGHT];

            if (
              (diff_err > -kAuto_align_max_diff && diff_err < kAuto_align_max_diff) &&
              (diff_err <= -kAuto_align_min_diff || diff_err >= kAuto_align_min_diff)
            ) {
              start_align(3);
            }
          } else if (
            sensor_distances[LEFT_FRONT] < kAuto_align_threshold &&
            sensor_distances[LEFT_REAR] < kAuto_align_threshold
          ) {
            int16_t diff_err = sensor_distances[LEFT_FRONT] - sensor_distances[LEFT_REAR];

            if (
              (diff_err > -kAuto_align_max_diff && diff_err < kAuto_align_max_diff) &&
              (diff_err <= -kAuto_align_min_diff || diff_err >= kAuto_align_min_diff)
            ) {
              start_align(2);
            }
          }

          // no alignment was started, move on to offset alignment
          if (state == IDLE) {
            Serial.println(F("Did not start dynamic alignment, moving to try forward alignment"));
            if (align_start_move_dir == FORWARD) {
              align_auto_state = ALIGN_AUTO_TURN;
            } else {
              align_auto_state = ALIGN_AUTO_IDLE;
            }
          }
        }
        break;
      case ALIGN_AUTO_TURN:
        if (state == IDLE) {
          if (
            sensor_obstacles[LEFT_FRONT] == sensor_obstacles[LEFT_REAR] &&
            sensor_obstacles[LEFT_FRONT] > -1 &&
            (sensor_obstacles[LEFT_FRONT] - 2) < kAuto_align_obstacle_target_length
          ) {
            align_dir = LEFT;
          } else if (solid_on_right >= 3) {
            align_dir = RIGHT;
          } else {
            Serial.println(F("Did not start turn because no suitable obstacles"));
            align_auto_state = ALIGN_AUTO_IDLE;
            break;
          }

          if (moves_since_turn_align >= kAuto_align_rate_limit) {
            moves_since_turn_align = 0;
          } else {
            align_auto_state = ALIGN_AUTO_IDLE;
            break;
          }

          // if both left sensors see an obstacle, we can turn and do a move-obstacle
          align_auto_state = ALIGN_AUTO_OBSTACLE;

          state = MOVE_COMMANDED;
          move_type = DISTANCE;
          move_dir = align_dir;
          move_align = false;
          move_report = false;

          target_left = unit_turn_to_ticks(2);
          target_right = unit_turn_to_ticks(2);

          if (align_dir == LEFT) {
            axis_left.setReverse(true);
            axis_right.setReverse(false);
          } else if (align_dir == RIGHT) {
            axis_left.setReverse(false);
            axis_right.setReverse(true);
          }
        }
        break;
      case ALIGN_AUTO_OBSTACLE:
        if (state == IDLE) {
          Serial.println(F("Start move obstacle for alignment"));

          state = MOVE_COMMANDED;
          move_type = OBSTACLE;
          move_dir = FORWARD;
          uint8_t blocks_away = sensor_distances[FRONT_FRONT_LEFT] / 100;
          if (blocks_away < kAuto_align_obstacle_target_length) {
            target_obstacle = kAuto_align_obstacle_targets[blocks_away];
          } else {
            Serial.println(F("Unexpected: we saw a wall before turning, but no wall after turning"));
            align_auto_state = ALIGN_AUTO_IDLE;
            break;
          }
          axis_left.setReverse(false);
          axis_right.setReverse(false);
        }
        break;
      case ALIGN_AUTO_UNTURN:
        if (state == IDLE) {
          align_auto_state = ALIGN_AUTO_DONE;

          Serial.println(F("Start unturn for alignment"));
          state = MOVE_COMMANDED;
          move_type = DISTANCE;
          move_dir = RIGHT;
          move_align = false;
          move_report = false;

          target_left = unit_turn_to_ticks(2);
          target_right = unit_turn_to_ticks(2);

          if (align_dir == LEFT) {
            axis_left.setReverse(false);
            axis_right.setReverse(true);
          } else if (align_dir == RIGHT) {
            axis_left.setReverse(true);
            axis_right.setReverse(false);
          }
        }
        break;
    }
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
  if (display.align_done) {
    Serial.println(F("align equal done"));
    display.align_done = 0;
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
  if (display.force_end) {
    Serial.println(F("force ended move"));
    display.force_end = 0;
  }

  if (state == REPORT_SENSOR && (millis() - report_delay_start) > kSensor_report_delay) {
    if (move_report) {
      log_all_sensors();

      moves_since_turn_align ++;

      if (move_dir == FORWARD) {
        if ((sensor_obstacles[RIGHT_FRONT] - 2) < kAuto_align_obstacle_target_length) {
          if (sensor_obstacles[RIGHT_FRONT] == solid_right_distance) {
            solid_on_right ++;
          } else {
            solid_right_distance = sensor_obstacles[RIGHT_FRONT];
            solid_on_right = 0;
          }
        }
      } else {
        solid_on_right = 0;
      }
    }

    if (move_align && align_auto_state == ALIGN_AUTO_IDLE) {
      Serial.println(F("Finished move, starting static alignment"));
      align_auto_state = ALIGN_AUTO_STATIC;
      align_start_move_dir = move_dir;
    } else if (align_auto_state == ALIGN_AUTO_STATIC) {
      // if we have just finished a distance for offset alignment, start dynamic
      Serial.println(F("Finished static alignment, starting dynamic alignment"));
      align_auto_state = ALIGN_AUTO_DYNAMIC;
    } else if (align_auto_state == ALIGN_AUTO_DONE) {
      align_auto_state = ALIGN_AUTO_IDLE;
    }

    state = IDLE;
  }
}
