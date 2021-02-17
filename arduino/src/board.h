#ifndef BOARD_H
#define BOARD_H

// both assumed to be in PCI0
#define E1A_PCINT PCINT19
#define E1B_PCINT PCINT21
#define E1_PIN PIND
#define E1A_BIT 3
#define E1B_BIT 5

// both assumed to be in PCI2
#define E2A_PCINT PCINT3
#define E2B_PCINT PCINT5
#define E2_PIN PINB
#define E2A_BIT 3
#define E2B_BIT 5

#define M1_INA_PORT PORTD
#define M1_INB_PORT PORTD
#define M1_INA_BIT 2
#define M1_INB_BIT 4

#define M2_INA_PORT PORTD
#define M2_INB_PORT PORTB
#define M2_INA_BIT 7
#define M2_INB_BIT 0

// map of sensor position to analog input
// the physical cables are labelled indexed from 1 while these are indexed from 0
// the enum values are <point_direction>_<position>
typedef enum {
  LEFT_FRONT = 4,
  FRONT_FRONT_LEFT = 3,
  FRONT_FRONT_MID = 0,
  FRONT_FRONT_RIGHT = 1,
  LEFT_REAR = 2,
  RIGHT_FRONT = 5,
} sensor_position_t;

#endif
