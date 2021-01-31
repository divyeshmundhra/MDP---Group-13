#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>
#include "config.h"

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

    int16_t getPower() {
      return _power;
    }

    void encoderEdge(int8_t delta);
    void controllerVelocity();

    void setVelocity(int16_t velocity) {
      _target_velocity = velocity;
    }
    int16_t getVelocity();
  private:
    /**
     * @brief Function pointer to be called to set motor PWM duty cycle
     */
    void (*_setPower)(uint16_t power, bool reverse);
    /**
     * @brief Updates target PWM duty cycle, then updates actual duty cycle after
     *  limiting maximum change with kMax_axis_accel/kMax_axis_decel
     * 
     * @param target_power New target power
     */
    void setPower(int16_t target_power);
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

    /**
     * @brief Time difference between the last two edges in microseconds
     * 
     */
    uint16_t _pulse_width = 0;
    /**
     * @brief Time at which the last edge was received in microseconds
     * 
     */
    uint32_t _last_edge = 0;
    /**
     * @brief Direction of last encoder edge
     * 
     */
    int8_t _last_encoder_dir = 0;
    /**
     * @brief Target velocity
     * 
     */
    int16_t _target_velocity = 0;
};

#endif
