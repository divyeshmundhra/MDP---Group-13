#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(void (*setSpeed)(uint16_t speed, bool reverse)): _setSpeed(setSpeed) {};
    /**
     * @brief Construct a new Axis object
     * 
     * @param setSpeed function pointer to be called to actually set speed of this axis
     * @param invert whether this axis is inverted
     */
    Axis(void (*setSpeed)(uint16_t speed, bool reverse), bool invert): _setSpeed(setSpeed), _invert(invert) {};
    /**
     * @brief Updates target PWM duty cycle, then updates actual duty cycle after
     *  limiting maximum change with kMax_axis_accel/kMax_axis_decel
     * 
     * @param target_speed New target speed
     */
    void setSpeed(int16_t target_speed);
    /**
     * @brief Invert the axis temporarily
     * 
     * @param reverse true to invert, or false otherwise
     */
    void setReverse(bool reverse) {
      _reverse = reverse;
    }

    /**
     * @brief Invert the encoder value if the axis is currently reversed
     * 
     * @param count encoder count
     * @return int32_t corrected encoder count
     */
    int32_t readEncoder(int32_t count) {
      if (_reverse) {
        return -count;
      }

      return count;
    }

    int16_t getSpeed() {
      return _speed;
    }
  private:
    /**
     * @brief Function pointer to be called to set motor PWM duty cycle
     */
    void (*_setSpeed)(uint16_t speed, bool reverse);
    /**
     * @brief Actual duty cycle written to the motor
     */
    int16_t _speed = 0;
    /**
     * @brief Target duty cycle
     */
    int16_t _target_speed = 0;

    /**
     * @brief Whether this axis should be inverted
     * 
     */
    bool _invert = false;
    /**
     * @brief Whether this axis is temporarily inverted for moving in reverse
     * 
     */
    bool _reverse = false;
};

#endif
