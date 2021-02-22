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
const uint16_t kTicks_per_45_degrees = 772;

// distance between sideways front and rear sensors
// (distance sensor to standoff) + (distance standoff to standoff) + (distance standoff to sensor)
const uint16_t kSensor_side_front_rear_distance = 15 + 145 + 15;
const uint16_t kSensor_side_center_distance = kSensor_side_front_rear_distance / 2;

const uint16_t kBlock_distance = distanceToTicks(100);

// distance of travel after which to report sensor values
const uint16_t kReport_distance = 0.95 * kBlock_distance;
// delay the sensor read until the middle of the block to get a more accurate reading
// this is not added for the final block since we won't actually move until the end of the block
const uint16_t kReport_distance_offset = 0.4 * kBlock_distance;

#endif
