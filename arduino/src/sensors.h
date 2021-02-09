#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

void setup_sensors();
void loop_sensors();
void log_sensor(uint8_t i);

void convert_sensor_data();
extern volatile int16_t sensor_distances[6];

#endif
