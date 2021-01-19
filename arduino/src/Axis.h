#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd)): _setSpeed(setSpeed) {};
    Axis(void (*setSpeed)(uint16_t speed, uint8_t fwd), bool reverse): _setSpeed(setSpeed), _reverse(reverse) {};
    inline void encoderEdge(uint8_t state) {
      /*
        Encoder state machine from
        http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
      */
      const int8_t _kEncoder_LUT[16] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
      static uint8_t enc_val = 0;
      enc_val = ((enc_val << 2) | state) & 0x0F;
      if (!_reverse) {
        _encoder_count += _kEncoder_LUT[enc_val];
      } else {
        _encoder_count -= _kEncoder_LUT[enc_val];
      }
    };
    void setTargetSpeed(int16_t speed) {
      _target_speed = speed;
      updateSpeed();
    };
    void updateSpeed();

    int16_t getCurSpeed() {
      return _cur_speed;
    }

    int32_t getEncoderCount() {
      return _encoder_count;
    }
  private:
    volatile int32_t _encoder_count = 0;
    void (*_setSpeed)(uint16_t speed, uint8_t fwd);
    int16_t _cur_speed = 0;
    int16_t _target_speed = 0;

    bool _reverse = false;
};

#endif
