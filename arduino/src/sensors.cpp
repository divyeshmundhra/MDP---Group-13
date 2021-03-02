#include <Arduino.h>
#include "sensors.h"
#include "board.h"
#include "config.h"

volatile uint16_t adc_val[6] = {0};
volatile int16_t sensor_distances[6] = {0};
volatile int8_t sensor_obstacles[6] = {0};

ISR(ADC_vect) {
  // channel represents the channel of this conversion (the trigger for this ISR)
  static uint8_t channel = 0;
  // represents the channel of the next conversion
  static uint8_t next_channel = 0;

  // use the decimation method described at http://ww1.microchip.com/downloads/en/Appnotes/doc8003.pdf
  static uint16_t sum[6] = {0};
  static uint8_t sum_count[6] = {0};

  uint16_t new_val = ADCL;
  new_val |= ADCH << 8;

  sum[channel] += new_val;
  sum_count[channel] ++;

  // we take 4^3 samples, giving us a 13 bit result
  if (sum_count[channel] >= 64) {
    adc_val[channel] = adc_val[channel] = ((uint32_t) kSensor_filter_alpha * (sum[channel] >> 3) + (uint32_t) (255 - kSensor_filter_alpha) * adc_val[channel]) >> 8;;
    sum[channel] = 0;
    sum_count[channel] = 0;
  }

  channel = next_channel;
  next_channel ++;
  if (next_channel >= 6) {
    next_channel = 0;
  }

  ADMUX = (ADMUX & 0xF0) | next_channel; // select next channel
  ADCSRA |= _BV(ADSC);              // start conversion
}

void setup_sensors() {
  // using external AREF (3.3V) by default
  /*
    ADEN: enable ADC
    ADATE: auto trigger (ADCSRB ADTS[2:0] defaults to 0, free running mode)
    ADIE: enable ADC interrupt
    ADPS[2:0]: prescalar of 128 (ADC clock of 125kHz)
  */
  ADCSRA |= _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
  ADCSRA |= _BV(ADSC); // start conversion
}

void convert_sensor_data() {
  for(uint8_t i = 0; i < 6; i++) {
    sensor_distances[i] = (kSensor_constants[i][0] * adc_val[i] + kSensor_constants[i][1]) / (adc_val[i] + kSensor_constants[i][2]);
    sensor_distances[i] += kSensor_offset[i];

    if (sensor_distances[i] < 0) {
      sensor_distances[i] = 0;
    }

    sensor_obstacles[i] = -1;

    if (sensor_distances[i] > kSensor_max[i]) {
      continue;
    }

    for (uint8_t u = 0; u < kSensor_threshold_count; u++) {
      if (sensor_distances[i] < kSensor_thresholds[i][u]) {
        sensor_obstacles[i] = 2 + u;
        break;
      }
    }
  }
}

bool log_sensors = false;
void loop_sensors() {
  if (log_sensors) {
    static uint32_t last_log = 0;
    uint32_t cur_time = millis();
    
    if ((cur_time - last_log) > 10) {
      Serial.print("SYNC");
      cli();
      for(uint8_t i = 0; i < 6; i++) {
        Serial.write((char *) &sensor_distances[i], 2);
      }
      sei();
      Serial.println();

      last_log = cur_time;
    }
  }
}

void log_sensor(uint8_t i) {
  if (i == 0 || i > 16) {
    return;
  }

  if (i >= 11 && i <= 16) {
    Serial.println(adc_val[i - 10 - 1]);
    return;
  }

  Serial.print("Sensor ");
  Serial.print(i);
  Serial.print(": raw=");
  cli();
  Serial.print(adc_val[i - 1]);
  Serial.print(" actual=");
  Serial.print(sensor_distances[i - 1]);
  Serial.print(" obstacle=");
  Serial.println(sensor_obstacles[i - 1]);
  sei();
}

void log_all_sensors() {
  Serial.print("$SENSOR ");
  for(uint8_t i = 0; i < 6; i++) {
    if (sensor_obstacles[i] == -1) {
      Serial.print("i");
    } else {
      Serial.print(sensor_obstacles[i]);
    }
    Serial.print("|");
  }
  Serial.println();
}

void log_sensor_nicely(int16_t i) {
  if (i != -1) {
    Serial.print(i, HEX);
  } else {
    Serial.print("i");
  }
}

void log_all_sensors_art() {
  /*
    logs sensors relative to their physical position like
    1a5
    2 5
       
    2
  */
  Serial.println("Sensors:");

  log_sensor_nicely(sensor_obstacles[FRONT_FRONT_LEFT]);
  log_sensor_nicely(sensor_obstacles[FRONT_FRONT_MID]);
  log_sensor_nicely(sensor_obstacles[FRONT_FRONT_RIGHT]);
  Serial.println();

  log_sensor_nicely(sensor_obstacles[LEFT_FRONT]);
  Serial.print(" ");
  log_sensor_nicely(sensor_obstacles[RIGHT_FRONT]);

  Serial.println("   ");
  log_sensor_nicely(sensor_obstacles[LEFT_REAR]);
  Serial.println();
}
