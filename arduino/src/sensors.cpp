#include <Arduino.h>
#include "sensors.h"
#include "board.h"
#include "config.h"

volatile uint16_t adc_val[6] = {0};
volatile uint16_t sensor_distances[6] = {0};

ISR(ADC_vect) {
  // channel represents the channel of this conversion (the trigger for this ISR)
  static uint8_t channel = 0;
  // represents the channel of the next conversion
  static uint8_t next_channel = 0;

  uint16_t new_val = ADCL;
  new_val |= ADCH << 8;

  adc_val[channel] = ((uint32_t) kSensor_filter_alpha * new_val + (uint32_t) (255 - kSensor_filter_alpha) * adc_val[channel]) >> 8;

  channel = next_channel;
  next_channel ++;
  if (next_channel >= 6) {
    next_channel = 0;
  }

  ADMUX = (ADMUX & 0xF0) | next_channel; // select next channel
  ADCSRA |= _BV(ADSC);              // start conversion
}

void setup_sensors() {
  ADMUX |= _BV(REFS0); // Voltage Reference = AVcc
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
  sensor_distances[FRONT_MID] = (kSensor_constants[FRONT_MID][0] * adc_val[FRONT_MID] + kSensor_constants[FRONT_MID][1]) / (adc_val[FRONT_MID] + kSensor_constants[FRONT_MID][2]);
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
