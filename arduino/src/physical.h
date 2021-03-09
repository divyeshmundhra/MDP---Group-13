#ifndef PHYSICAL_H
#define PHYSICAL_H

// encoder counts per revolution of the motor output
const uint16_t kCount_per_rev = 2249;

// wheel diameter (mm)
const uint16_t kWheel_diameter = 60;

// center-to-center distance (mm) between the two wheels
const uint16_t kWheel_distance = 172;

const uint16_t kWheel_circumference = 3.14159 * kWheel_diameter;
const uint16_t kRobot_circumference = 3.14159 * kWheel_distance;

#define distanceToTicks(distance) (((uint32_t) distance) * kCount_per_rev / kWheel_circumference)
// #define angleToDistance(angle) (((uint32_t) angle) * kRobot_circumference / 360)
// convert directly to reduce impact of rounding
#define angleToTicks(angle) (((uint32_t) angle) * kWheel_distance * kCount_per_rev / (360 * kWheel_diameter))

// hardcode ticks per 45 degree rotation to allow for tuning
const uint16_t kTicks_per_45_degrees = 818;

// ticks to turn 45 degrees when F is combined with L/R
const uint16_t kTicks_per_45_degrees_combined = 1500;

// distance between sideways front and rear sensors
// (distance sensor to standoff) + (distance standoff to standoff) + (distance standoff to sensor)
const uint16_t kSensor_side_front_rear_distance = 15 + 145 + 15;
const uint16_t kSensor_side_center_distance = kSensor_side_front_rear_distance / 2;

const uint16_t kBlock_distance = 1195;

#endif
