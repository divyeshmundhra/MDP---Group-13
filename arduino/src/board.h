#ifndef BOARD_H
#define BOARD_H

// both assumed to be in PCI0
#define E1A_PCINT PCINT19
#define E1B_PCINT PCINT21
#define E1_PIN PIND
#define E1A_BIT 3
#define E1B_BIT 5
// format 0b00B0 A000 into 0b0000 00AB
#define E1_FLUSH_RIGHT (((E1_PIN & _BV(E1A_BIT)) >> 2) | ((E1_PIN & _BV(E1B_BIT)) >> 5))

// both assumed to be in PCI2
#define E2A_PCINT PCINT3
#define E2B_PCINT PCINT5
#define E2_PIN PINB
#define E2A_BIT 3
#define E2B_BIT 5
#define E2_FLUSH_RIGHT (((E2_PIN & _BV(E2A_BIT)) >> 2) | ((E2_PIN & _BV(E2B_BIT)) >> 5))

#define M1_INA_PORT PORTD
#define M1_INB_PORT PORTD
#define M1_INA_BIT 2
#define M1_INB_BIT 4

#define M2_INA_PORT PORTD
#define M2_INB_PORT PORTB
#define M2_INA_BIT 7
#define M2_INB_BIT 0

#endif
