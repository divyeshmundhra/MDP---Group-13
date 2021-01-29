#include <Arduino.h>
#include "parser.h"
#include "motion.h"
#include "checklist.h"
#include "physical.h"
#include "config.h"

static char buf[kParser_buf_size + 1]; // + 1 for NULL character
static uint8_t buf_count = 0;

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
    start_motion_distance(FORWARD, distanceToTicks(val));
  } else if (cmd == 'B') {
    start_motion_distance(REVERSE, distanceToTicks(val));
  } else if (cmd == 'L') {
    start_motion_distance(LEFT, distanceToTicks(angleToDistance(val)));
  } else if (cmd == 'R') {
    start_motion_distance(RIGHT, distanceToTicks(angleToDistance(val)));
  } else if (cmd == 'O') {
    start_motion_obstacle(val);
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
  }
  else if (cmd == 'A') {
    start_checklist(val);
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
        printf("buffer full\n");
        buf_count = 0;
      }

      buf[buf_count++] = c;
    }
  }
}
