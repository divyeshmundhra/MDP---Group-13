#include <Arduino.h>
#include "parser.h"
#include "motion.h"
#include "config.h"

static char buf[kParser_buf_size + 1]; // + 1 for NULL character
static uint8_t buf_count = 0;

static bool parse_buf() {
  char cmd = buf[0];
  char *next;
  uint32_t val = strtoul(&buf[1], &next, 10);

  // *next should be pointing to the NULL terminator if the string was parsed successfully
  if (*next != '\0') {
    return false;
  }

  if (cmd == 'F') {
    start_motion(FORWARD, val);
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
