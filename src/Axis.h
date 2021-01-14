#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    void encoderEdge();

    uint32_t getEncoderCount() {
      return _encoder_count;
    }
  private:
    volatile uint32_t _encoder_count = 0;
};

#endif
