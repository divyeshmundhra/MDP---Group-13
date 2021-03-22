#ifndef ALIGN_LUT_H
#define ALIGN_LUT_H

/*
import math

kCount_per_rev = 2249
kWheel_diameter = 60
kWheel_distance = 175
kSensor_side_front_rear_distance = 15 + 145 + 15
kSensor_front_distance = -8 + 175 + -8

gen_count = 64

kWheel_circumference = math.pi * kWheel_diameter
kRobot_circumference = math.pi * kWheel_distance
distanceToTicks = lambda distance: distance * kCount_per_rev / kWheel_circumference
angleToDistance = lambda angle: angle * kRobot_circumference / 360

def generate_distances(sensor_distance, count):
    return list([int(distanceToTicks(angleToDistance(math.asin(i/sensor_distance) / math.pi * 180))) for i in range(count)])

left_lut = str(generate_distances(kSensor_side_front_rear_distance, gen_count))[1:-1]
front_lut = str(generate_distances(kSensor_front_distance, gen_count))[1:-1]

print(f'const uint8_t align_LUT_len = {gen_count};')
print(f'const PROGMEM uint16_t align_left_LUT[] = {{{left_lut}}};')
print(f'const PROGMEM uint16_t align_forward_LUT[] = {{{front_lut}}};')
*/

// maps left sensor delta into number of encoder ticks to turn to correct it
const uint8_t align_LUT_len = 64;
const PROGMEM uint16_t align_left_LUT[] = {0, 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 83, 89, 95, 101, 107, 113, 119, 125, 131, 137, 143, 149, 155, 161, 167, 173, 179, 185, 191, 198, 204, 210, 216, 222, 228, 234, 240, 246, 253, 259, 265, 271, 277, 283, 290, 296, 302, 308, 314, 321, 327, 333, 340, 346, 352, 359, 365, 371, 378, 384};
const PROGMEM uint16_t align_forward_LUT[] = {0, 6, 13, 19, 26, 32, 39, 45, 52, 59, 65, 72, 78, 85, 92, 98, 105, 111, 118, 125, 131, 138, 144, 151, 158, 164, 171, 178, 184, 191, 198, 204, 211, 218, 224, 231, 238, 245, 251, 258, 265, 272, 279, 285, 292, 299, 306, 313, 320, 327, 333, 340, 347, 354, 361, 368, 375, 382, 389, 396, 403, 411, 418, 425};
#endif
