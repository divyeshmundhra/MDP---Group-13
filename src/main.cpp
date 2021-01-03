#include <Arduino.h>
#include <avr/io.h>

volatile uint16_t count_left = 0;
volatile uint16_t count_right = 0;

ISR(INT1_vect) {
  count_left ++;
}

ISR(PCINT0_vect) {
  // this ISR does not check for which pin triggered the ISR
  // as only PCINT3 is activated

  // if pin is currently high, this is a rising edge
  if (PINB & _BV(PCINT3)) {
    count_right ++;
  }
}

void setup() {
  // ISR init for encoder:

  // INT1 (Motor A encoder):
  EICRA |= _BV(ISC11) | _BV(ISC10); // INT1 trigger on rising edge
  EIFR &= ~_BV(INTF1);              // clear interrupt flag of INT1
  EIMSK |= _BV(INT1);               // enable INT1

  // PCINT3 (Motor B encoder):
  PCMSK0 |= _BV(PCINT3);            // enable PCINT3 
  PCIFR &= ~_BV(PCIF0);             // clear interrupt flag of PCI0
  PCICR |= _BV(PCIE0);              // enable PCI0

  Serial.begin(115200);
}

void loop() {
  static uint32_t last_print = millis();
  uint32_t cur_time = millis();

  if ((cur_time - last_print) > 100) {
    cli();
    Serial.print(count_left);
    Serial.print(", ");
    Serial.println(count_right);

    count_left = 0;
    count_right = 0;

    sei();
    last_print = cur_time;
  }
}
