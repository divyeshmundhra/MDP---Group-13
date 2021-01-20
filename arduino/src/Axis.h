#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd)): _setSpeed(setSpeed) {};
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd), bool reverse): _setSpeed(setSpeed), _reverse(reverse) {};
    void setSpeed(int16_t speed);

    int16_t getSpeed() {
      return _speed;
    }
  private:
    void (*_setSpeed)(uint16_t speed, uint8_t fwd);
    int16_t _speed = 0;

    bool _reverse = false;
};

#endif
