#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(uint16_t _pid_p, uint16_t _pid_i, uint16_t _pid_d): pid_p(_pid_p), pid_i(_pid_i), pid_d(_pid_d) {};

    void encoderEdge();
    void controller();

    int16_t getPower() {
      return _power;
    }

    int16_t getError() {
      return _power;
    }

    uint16_t getPulseWidth() {
      return _pulse_width;
    }
  private:
    const uint16_t pid_p, pid_i, pid_d;

    volatile uint16_t _encoder_count = 0;
    volatile uint32_t _last_pulse = 0;
    volatile uint16_t _pulse_width = 0;

    volatile int16_t _power = 0;
    volatile int16_t _integral = 0;
    volatile int16_t _last_error = 0;
    volatile int16_t _error = 0;
};

#endif
