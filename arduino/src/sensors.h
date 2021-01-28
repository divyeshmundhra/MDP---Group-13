#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

void setup_sensors();
void loop_sensors();

void convert_sensor_data();
extern volatile uint16_t sensor_distances[6];

#endif
