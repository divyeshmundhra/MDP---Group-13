#include <Arduino.h>
#include "sensors.h"
#include "board.h"
#include "config.h"

volatile uint16_t adc_val[6] = {0};
volatile uint16_t filtered_adc_val[6] = {0};
int16_t sensor_distances[6] = {0};
int16_t filtered_sensor_distances[6];
int8_t sensor_obstacles[6] = {0};

bool sensor_stable[6] = {0};

volatile bool has_new_val[6] = {0};

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
    uint16_t val = sum[channel] >> 3;

    adc_val[channel] = val;
    filtered_adc_val[channel] = ((uint32_t) kSensor_filter_alpha * val + (uint32_t) (255 - kSensor_filter_alpha) * adc_val[channel]) >> 8;
    sum[channel] = 0;
    sum_count[channel] = 0;

    has_new_val[channel] = true;
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
  static int16_t pSensor_distances[6] = {0};

  for(uint8_t i = 0; i < 6; i++) {
    if (!has_new_val[i]) {
      continue;
    }

    pSensor_distances[i] = sensor_distances[i];

    if (adc_val[i] < kSensor_min_value) {
      sensor_distances[i] = INT16_MAX;
      sensor_obstacles[i] = -1;
      continue;
    }

    cli();
    int16_t val = adc_val[i];
    int16_t filtered_val = filtered_adc_val[i];
    has_new_val[i] = false;
    sei();

    sensor_distances[i] = (kSensor_constants[i][0] * val + kSensor_constants[i][1]) / (val + kSensor_constants[i][2]);
    sensor_distances[i] += kSensor_offset[i];

    int16_t distance = (kSensor_constants[i][0] * filtered_val + kSensor_constants[i][1]) / (filtered_val + kSensor_constants[i][2]);
    distance += kSensor_offset[i];

    int16_t delta = sensor_distances[i] - pSensor_distances[i];
    sensor_stable[i] = (delta > -kSensor_stable_threshold) && (delta < kSensor_stable_threshold);

    if (sensor_distances[i] < 0) {
      sensor_distances[i] = 0;
    }

    if (distance < 0) {
      distance = 0;
    }

    sensor_obstacles[i] = -1;

    if (distance > kSensor_max[i]) {
      continue;
    }

    for (uint8_t u = 0; u < kSensor_threshold_count; u++) {
      if (distance < kSensor_thresholds[i][u]) {
        sensor_obstacles[i] = 2 + u;
        break;
      }
    }
  }
}

bool log_sensors = false;
void loop_sensors() {
  static uint32_t last_convert = 0;
  uint32_t cur_time = millis();

  if ((cur_time - last_convert) > 10) {
    convert_sensor_data();
    last_convert = cur_time;
  }

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

void log_sensor_nicely(int8_t i) {
  if (i == -1) {
    Serial.write('i');
  } else if (i < 10) {
    Serial.write(0x30 + i);
  } else {
    // print as hex
    // 55 + 10 corresponds to A, 55 + 11 = B, ...
    Serial.write(55 + i);
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
  Serial.write('\n');

  log_sensor_nicely(sensor_obstacles[LEFT_FRONT]);
  Serial.write(' ');
  log_sensor_nicely(sensor_obstacles[RIGHT_FRONT]);

  Serial.println("   ");
  log_sensor_nicely(sensor_obstacles[LEFT_REAR]);
  Serial.write('\n');
}
