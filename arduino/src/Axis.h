#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>

class Axis {
  public:
    Axis(void (*setPower)(uint16_t power, bool reverse)): _setPower(setPower) {};
    /**
     * @brief Construct a new Axis object
     * 
     * @param setPower function pointer to be called to actually set power of this axis
     * @param invert whether this axis is inverted
     */
    Axis(void (*setPower)(uint16_t power, bool reverse), bool invert): _setPower(setPower), _invert(invert) {};
    /**
     * @brief Updates target PWM duty cycle, then updates actual duty cycle after
     *  limiting maximum change with kMax_axis_accel/kMax_axis_decel
     * 
     * @param target_power New target power
     */
    void setPower(int16_t target_power);
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
    int32_t getEncoder() {
      if (_invert ^ _reverse) {
        return -encoder_count;
      }

      return encoder_count;
    }

    void resetEncoder() {
      encoder_count = 0;
    }

    volatile int32_t encoder_count = 0;

    int16_t getPower() {
      return _power;
    }
  private:
    /**
     * @brief Function pointer to be called to set motor PWM duty cycle
     */
    void (*_setPower)(uint16_t power, bool reverse);

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
};

#endif
