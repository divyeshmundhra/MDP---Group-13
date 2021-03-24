#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>
#include "config.h"

class Axis {
  public:
    Axis(void (*setPower)(uint16_t power, bool reverse), void (*setBrake)(uint16_t power)): _setPower(setPower), _setBrake(setBrake) {};
    /**
     * @brief Construct a new Axis object
     * 
     * @param setPower function pointer to be called to actually set power of this axis
     * @param invert whether this axis is inverted
     */
    Axis(void (*setPower)(uint16_t power, bool reverse), void (*setBrake)(uint16_t power), bool invert): _setPower(setPower), _setBrake(setBrake), _invert(invert) {};
    /**
     * @brief Updates target PWM duty cycle, then updates actual duty cycle after
     *  limiting maximum change with kMax_axis_accel/kMax_axis_decel
     * 
     * @param target_power New target power
     * @param cap_accel Whether to limit acceleration
     */
    void setPower(int16_t target_power, bool cap_accel);

    void setBrake(uint16_t power) {
      _setBrake(power);
    }
    /**
     * @brief Invert the axis temporarily
     * 
     * @param reverse true to invert, or false otherwise
     */
    void setReverse(bool reverse);

    void encoderEdge(int8_t delta) {
      if (
        (_encoder_correction > 0 && delta < 0) ||
        (_encoder_correction < 0 && delta > 0)
      ) {
        // we add delta only if it will cancel out _encoder_correction, else we add it to the main count
        // this prevents _encoder_correction from growing too large in one direction and overflowing
        _encoder_correction += delta;
      } else {
        _encoder_count += delta;
      }
    }

    /**
     * @brief Invert the encoder value if the axis is currently reversed
     * 
     * @param count encoder count
     * @return int32_t corrected encoder count
     */
    int32_t getEncoder() {
      if (_invert ^ _reverse) {
        return -_encoder_count - _encoder_correction;
      }

      return _encoder_count + _encoder_correction;
    }

    void resetEncoder() {
      _encoder_count = 0;
      _encoder_correction = 0;
    }

    void resetEncoderForNextMove(int32_t error);

    void incrementEncoder(int32_t delta) {
      if (_invert) {
        _encoder_correction -= delta;
      } else {
        _encoder_correction += delta;
      }

      if (_encoder_correction < kMin_encoder_correction) {
        _encoder_correction = kMin_encoder_correction;
      } else if (_encoder_correction > kMax_encoder_correction) {
        _encoder_correction = kMax_encoder_correction;
      }
    }

    int16_t getPower() {
      return _power;
    }
  private:
    /**
     * @brief Function pointer to be called to set motor PWM duty cycle
     */
    void (*_setPower)(uint16_t power, bool reverse);

    void (*_setBrake)(uint16_t power);

    /**
     * @brief Actual duty cycle written to the motor
     */
    int16_t _power = 0;
    /**
     * @brief Target duty cycle
     */
    int16_t _target_power = 0;

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

    volatile int32_t _encoder_count = 0;
    volatile int16_t _encoder_correction = 0;
};

#endif
