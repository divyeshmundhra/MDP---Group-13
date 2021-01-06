#ifndef BOARD_H
#define BOARD_H

// both assumed to be in PCI0
#define PIN_ENCODER_LEFT1 PCINT19
#define PIN_ENCODER_LEFT2 PCINT21

// both assumed to be in PCI2
#define PIN_ENCODER_RIGHT1 PCINT3
#define PIN_ENCODER_RIGHT2 PCINT5

#define PIN_MD_EN 6
#define PIN_SW 14

#define TIME_DDR DDRC
#define TIME_PORT PORTC
#define TIME_BIT 1

#endif
