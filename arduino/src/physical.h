#ifndef PHYSICAL_H
#define PHYSICAL_H

// encoder counts per revolution of the motor output
const uint16_t kCount_per_rev = 2249;

// wheel diameter (mm)
const uint16_t kWheel_diameter = 60;

// center-to-center distance (mm) between the two wheels
const uint16_t kWheel_distance = 175;

const uint16_t kWheel_circumference = 3.14159 * kWheel_diameter;
const uint16_t kRobot_circumference = 3.14159 * kWheel_distance;

#define distanceToTicks(distance) (((uint32_t) distance) * kCount_per_rev / kWheel_circumference)
#define angleToDistance(angle) (((uint32_t) angle) * kRobot_circumference / 360)

const uint16_t kBlock_distance = distanceToTicks(100);

#endif
