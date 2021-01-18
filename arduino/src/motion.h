#ifndef MOTION_H
#define MOTION_H

typedef enum {
  FORWARD,
  REVERSE,
  LEFT,
  RIGHT
} motion_direction_t;

void setup_motion();
void loop_motion();

void start_motion(motion_direction_t direction, uint32_t distance);

#endif
