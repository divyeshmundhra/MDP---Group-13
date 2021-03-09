#include <Arduino.h>
#include "parser.h"
#include "motion.h"
#include "physical.h"
#include "sensors.h"
#include "config.h"
#include "board.h"

static char buf[kParser_buf_size + 1]; // + 1 for NULL character
static uint8_t buf_count = 0;

typedef enum {
  DEBUG_IDLE,
  DEBUG_LOG_SPECIFIC_SENSOR,
  DEBUG_LOG_ALL_SENSORS
} debug_state_t;

debug_state_t state = DEBUG_IDLE;
uint8_t debug_sensor_target = 0;

static bool parse_buf() {
  char cmd = buf[0];
  char cmd1 = buf[1];
  char *next;
  uint32_t val;

  if (cmd1 >= '0' && cmd1 <= '9') {
    val = strtoul(&buf[1], &next, 10);
  } else {
    val = strtoul(&buf[2], &next, 10);
  }

  // *next should be pointing to the NULL terminator if the string was parsed successfully
  if (*next != '\0') {
    return false;
  }

  if (cmd == 'F') {
    start_motion_unit(FORWARD, val);
  } else if (cmd == 'B') {
    start_motion_unit(REVERSE, val);
  } else if (cmd == 'L') {
    start_motion_unit(LEFT, val);
  } else if (cmd == 'R') {
    start_motion_unit(RIGHT, val);
  } else if (cmd == 'O') {
    start_motion_obstacle(val);
  } else if (cmd == 'l') {
    if (cmd1 == 'r') {
      start_motion_distance(LEFT, val);
    } else {
      start_motion_distance(LEFT, angleToTicks(val));
    }
  } else if (cmd == 'r') {
    if (cmd1 == 'r') {
      start_motion_distance(RIGHT, val);
    } else {
      start_motion_distance(RIGHT, angleToTicks(val));
    }
  } else if (cmd == 'Q') {
    if (cmd1 == 'A') {
      log_all_sensors_art();
    } else if (cmd1 == 'a') {
      log_all_sensors();
    } else {
      log_sensor(val);
    }
  } else if (cmd == 'A') {
    start_align(val);
  } else if (cmd == 'D') {
    state = DEBUG_IDLE;
  } else if (cmd == 'S') {
    if (cmd1 == 'A') {
      state = DEBUG_LOG_ALL_SENSORS;
    } else {
      state = DEBUG_LOG_SPECIFIC_SENSOR;
      debug_sensor_target = val;
    }
  } else if (cmd == 'E') {
    if (cmd1 == 'm') {
      log_motion = !log_motion;
      Serial.print("log motion: ");
      Serial.println(log_motion);
    } else if (cmd1 == 's') {
      log_sensors = !log_sensors;
      Serial.print("log sensors: ");
      Serial.println(log_sensors);
    } else if (cmd1 == 'p') {
      parse_moves = !parse_moves;
      Serial.print("parse moves: ");
      Serial.println(parse_moves);
    }
  } else if (cmd == 'o') {
    if (cmd1 == 'p') {
      kP_offset = val;
    } else if (cmd1 == 'i') {
      kI_offset = val;
    } else if (cmd1 == 'd') {
      kD_offset = val;
    }
  } else if (cmd == 's') {
    if (cmd1 == 'p') {
      kP_straight = val;
    } else if (cmd1 == 'i') {
      kI_straight = val;
    } else if (cmd1 == 'd') {
      kD_straight = val;
    }
  } else if (cmd == 'b') {
    if (cmd1 == 'p') {
      kP_obstacle = val;
    } else if (cmd1 == 'i') {
      kI_obstacle = val;
    } else if (cmd1 == 'd') {
      kD_obstacle = val;
    }
  } else if (cmd == 'p') {
    if (cmd1 == 'a') {
      kP_wall_diff_left = val;
    } else if (cmd1 == 'b') {
      kP_wall_offset_left = val;
    } else if (cmd1 == 'c') {
      kP_wall_diff_forward = val;
    } else if (cmd1 == 'd') {
      kP_wall_offset_right = val;
    }
  } else if (cmd == 'w') {
    if (cmd1 == 'p') {
      kP_align = val;
    } else if (cmd1 == 'i') {
      kI_align = val;
    } else if (cmd1 == 'd') {
      kD_align = val;
    }
  } else {
    Serial.println("Unknown cmd");
  }

  return true;
}

void setup_parser() {
  
}

void loop_parser() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == '\r') {
      return;
    } else if (c == '\n') {
      if (buf_count < 2) {
        Serial.println("error: too short\n");
        buf_count = 0;
        return;
      }

      buf[buf_count] = '\0';

      if (parse_buf()) {
        Serial.println("ok");
      } else {
        Serial.println("err");
      }

      buf_count = 0;
    } else {
      if (buf_count >= kParser_buf_size) {
        Serial.println("buffer full");
        buf_count = 0;
      }

      buf[buf_count++] = c;
    }
  }

  static uint32_t last_log = 0;
  uint32_t cur = millis();

  if ((cur - last_log) > 10) {
    last_log = cur;

    switch (state) {
      case DEBUG_LOG_SPECIFIC_SENSOR:
        log_sensor(debug_sensor_target);
        break;
      case DEBUG_LOG_ALL_SENSORS:
        log_all_sensors();
        break;
      default:
        break;
    }
  }
}
