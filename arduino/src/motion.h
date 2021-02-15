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

/**
 * @brief Moves the robot a set unit:
 *        Forward/Reverse: blocks, ie send 1 or 3 to move 1 or 3 blocks respectively
 *        Left/Right: 45 degree increments, ie send 1 or 2 to turn 45 or 90 degrees respectively
 * 
 * @param _direction 
 * @param unit 
 */
void start_motion_unit(motion_direction_t _direction, uint8_t unit);

void start_align();

bool get_motion_done();
int32_t get_encoder_left();

#endif
