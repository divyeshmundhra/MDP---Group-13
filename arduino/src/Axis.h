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
    void (*_setSpeed)(uint16_t speed, bool reverse);
    int16_t _speed = 0;
    int16_t _target_speed = 0;

    bool _invert = false;
    bool _reverse = false;
};

#endif
