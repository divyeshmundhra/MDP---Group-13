#ifndef ALIGN_LUT_H
#define ALIGN_LUT_H

/*
import math

kCount_per_rev = 2249
kWheel_diameter = 60
kWheel_distance = 175
kSensor_side_front_rear_distance = 15 + 145 + 15

gen_count = 32

kWheel_circumference = math.pi * kWheel_diameter
kRobot_circumference = math.pi * kWheel_distance
distanceToTicks = lambda distance: distance * kCount_per_rev / kWheel_circumference
angleToDistance = lambda angle: angle * kRobot_circumference / 360

print(list([int(distanceToTicks(angleToDistance(math.asin(i/kSensor_side_front_rear_distance) / math.pi * 180))) for i in range(gen_count)]))
*/

// maps left sensor delta into number of encoder ticks to turn to correct it
const PROGMEM uint8_t align_LUT[] = { 0, 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 83, 89, 95, 101, 107, 113, 119, 125, 131, 137, 143, 149, 155, 161, 167, 173, 179, 185 };
const uint8_t align_LUT_len = sizeof(align_LUT) / sizeof(uint8_t);
#endif
