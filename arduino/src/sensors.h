#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

void setup_sensors();
void loop_sensors();
void log_sensor(uint8_t i);
void log_all_sensors();
void log_all_sensors_art();

void convert_sensor_data();
extern int16_t sensor_distances[6];
extern int8_t sensor_obstacles[6];
extern bool sensor_stable[6];

extern bool log_sensors;

#endif
