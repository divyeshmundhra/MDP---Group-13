#include <Arduino.h>
#include "sensors.h"
#include "config.h"

volatile uint16_t adc_val = 0;

ISR(ADC_vect) {
  uint16_t val = ADCL;
  val |= ADCH << 8;

  adc_val = ((uint32_t) kSensor_filter_alpha * val + (uint32_t) (255 - kSensor_filter_alpha) * adc_val) >> 8;
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

void loop_sensors() {
  cli();
  uint16_t _adc_val = adc_val;
  sei();

  Serial.print("SYNC");
  Serial.write((char *) &_adc_val, 2);
  delay(10);
}
