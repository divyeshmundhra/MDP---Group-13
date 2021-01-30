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

/**
 * @brief Start travel in \p direction for \p distance
 * 
 * @param direction Direction to move in
 * @param distance Distance to move for
 */
void start_motion_distance(motion_direction_t direction, uint32_t distance);
/**
 * @brief Start forward movement until obstacle is seen at \p distance
 * 
 * @param distance
 */
void start_motion_obstacle(uint16_t distance);
void set_speed(uint16_t speed);

bool get_motion_done();
int32_t get_encoder_left();

#endif
