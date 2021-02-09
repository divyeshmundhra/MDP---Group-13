#include <Arduino.h>
#include "sensors.h"
#include "board.h"
#include "config.h"

volatile uint16_t adc_val[6] = {0};
volatile int16_t sensor_distances[6] = {0};

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
    if (sensor_distances[i] < 0) {
      sensor_distances[i] = 0;
    }
  }
}

void loop_sensors() {
  #if 0
    Serial.print("SYNC");
    cli();
    for(uint8_t i = 0; i < 6; i++) {
      Serial.write((char *) &sensor_distances[i], 2);
    }
    sei();
    Serial.println();

    delay(10);
  #endif
}

void log_sensor(uint8_t i) {
  if (i == 0 || i > 6) {
    return;
  }

  Serial.print("Sensor ");
  Serial.print(i);
  Serial.print(": raw=");
  cli();
  Serial.print(adc_val[i - 1]);
  Serial.print(" actual=");
  Serial.println(sensor_distances[i - 1]);
  sei();
}
