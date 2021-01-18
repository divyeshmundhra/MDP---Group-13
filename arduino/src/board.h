#ifndef BOARD_H
#define BOARD_H

// both assumed to be in PCI0
#define PIN_ENCODER_LEFT1 PCINT19
#define PIN_ENCODER_LEFT2 PCINT21

// both assumed to be in PCI2
#define PIN_ENCODER_RIGHT1 PCINT3
#define PIN_ENCODER_RIGHT2 PCINT5

#define M1_INA_PORT PORTD
#define M1_INB_PORT PORTD
#define M1_INA_BIT 2
#define M1_INB_BIT 4

#define M2_INA_PORT PORTD
#define M2_INB_PORT PORTB
#define M2_INA_BIT 7
#define M2_INB_BIT 0

#endif
