#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd)): _setSpeed(setSpeed) {};
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd), bool reverse): _setSpeed(setSpeed), _reverse(reverse) {};
    void setTargetSpeed(int16_t speed) {
      _target_speed = speed;
      updateSpeed();
    };
    void updateSpeed();

    int16_t getCurSpeed() {
      return _cur_speed;
    }
  private:
    void (*_setSpeed)(uint16_t speed, uint8_t fwd);
    int16_t _cur_speed = 0;
    int16_t _target_speed = 0;

    bool _reverse = false;
};

#endif
