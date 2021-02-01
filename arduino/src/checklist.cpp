#include <stdint.h>
#include <Arduino.h>
#include "motion.h"
#include "physical.h"
#include "checklist.h"

typedef enum {
  CHECKLIST_NONE,
  CHECKLIST_A7
} checklist_active_t;

static checklist_active_t checklist_active = CHECKLIST_NONE;

void start_checklist(uint8_t number) {
  if (checklist_active != CHECKLIST_NONE) {
    Serial.println("Cannot start, checklist in progress");
    return;
  }

  switch(number) {
    case 7:
      checklist_a7(true);
      checklist_active = CHECKLIST_A7;
      break;
    default:
      Serial.println("Unsupported checklist item");
      break;
  }
}

void loop_checklist() {
  switch(checklist_active) {
    case CHECKLIST_A7:
      checklist_a7(false);
      break;
    default:
      break;
  }
}

typedef enum {
  A7_UNSET,
  A7_INIT,
  A7_MOVE_OBSTACLE,
  A7_45_RIGHT1,
  A7_45_RIGHT2,
  A7_DIAGONAL_MOVE1,
  A7_DIAGONAL_MOVE2,
  A7_90_LEFT,
  A7_MOVE_FORWARD,
  A7_DONE
} state_a7;

// distance (mm) to stop before obstacle
const uint16_t kA7_obstacle_distance_target = 125;
// distance (mm) to move overall
const uint16_t kA7_total_distance = 1500;
// distance (mm) offset after handling diagonal moves
const uint16_t kA7_distance_offset = 510;
// distance (mm) to move diagonally
const uint16_t kA7_diagonal_distance = 400;

void checklist_a7(bool reset) {
  static state_a7 state = A7_UNSET;
  static state_a7 state_prev = A7_UNSET;

  static int32_t distance_travelled = 0;

  state_a7 state_new = A7_UNSET;

  if (reset) {
    state = A7_INIT;
  }

  switch(state) {
    case A7_INIT:
      state_new = A7_MOVE_OBSTACLE;
      break;
    case A7_MOVE_OBSTACLE:
      if (state_prev != state) {
        Serial.print(state_prev);
        Serial.println("Enter A7_MOVE_OBSTACLE");
        start_motion_obstacle(kA7_obstacle_distance_target);
      }

      if (get_motion_done()) {
        state_new = A7_45_RIGHT1;
        distance_travelled = get_encoder_left();
        Serial.print("Distance travelled before obstacle: ");
        Serial.println(distance_travelled);
      }
      break;
    case A7_45_RIGHT1:
      if (state_prev != state) {
        Serial.println("Enter A7_45_RIGHT1");
        start_motion_distance(RIGHT, distanceToTicks(angleToDistance(45)));
      }

      if (get_motion_done()) {
        state_new = A7_DIAGONAL_MOVE1;
      }
      break;
    case A7_DIAGONAL_MOVE1:
      if (state_prev != state) {
        Serial.println("Enter A7_DIAGONAL_MOVE1");
        start_motion_distance(FORWARD, distanceToTicks(kA7_diagonal_distance));
      }

      if (get_motion_done()) {
        state_new = A7_90_LEFT;
      }
      break;
    case A7_90_LEFT:
      if (state_prev != state) {
        Serial.println("Enter A7_90_LEFT");
        start_motion_distance(LEFT, distanceToTicks(angleToDistance(90)));
      }

      if (get_motion_done()) {
        state_new = A7_DIAGONAL_MOVE2;
      }
      break;
    case A7_DIAGONAL_MOVE2:
      if (state_prev != state) {
        Serial.println("Enter A7_DIAGONAL_MOVE2");
        start_motion_distance(FORWARD, distanceToTicks(kA7_diagonal_distance));
      }

      if (get_motion_done()) {
        state_new = A7_45_RIGHT2;
      }
      break;
    case A7_45_RIGHT2:
      if (state_prev != state) {
        Serial.println("Enter A7_45_RIGHT2");
        start_motion_distance(RIGHT, distanceToTicks(angleToDistance(45)));
      }

      if (get_motion_done()) {
        state_new = A7_MOVE_FORWARD;
      }
      break;
    case A7_MOVE_FORWARD:
      if (state_prev != state) {
        Serial.println("Enter A7_MOVE_FORWARD");
        start_motion_distance(FORWARD, distanceToTicks(kA7_total_distance - kA7_distance_offset) - distance_travelled);
      }

      if (get_motion_done()) {
        state_new = A7_DONE;
      }
      break;
    default:
      break;
  }

  state_prev = state;

  if (state_new != A7_UNSET) {
    Serial.print("A7 state transition from ");
    Serial.print(state);
    Serial.print(" to ");
    Serial.println(state_new);

    state = state_new;
  }
}
