#include <stdlib.h>

#ifndef TASKMESSAGES_H
#define TASKMESSAGES_H

#define BUTTON_PRESS 1
#define BUTTON_RELEASE 2

typedef struct {
  int16_t   cmd1;
  int16_t   cmd2;
  int16_t   speed;
  float   batVoltage;
  int16_t   boardTemp;
  uint8_t   pushButtons;
  uint8_t   switchButtons;
  uint8_t   modeSwitch;
  uint8_t   buttonState;
} screendata_struct;

typedef struct {
  int16_t   x;
  int16_t   y;
  char msg[40];
  uint8_t fontnumber;
} screenDebug_struct;

#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_3 3
#define BUTTON_4 4
#define SWITCH_1 5
#define SWITCH_2 6
#define KNOB_1  7
#define KNOB_2  8
#define KNOB_3  9
#define KNOB_4 10
#define KNOB_5 11
#define BUTTON_EVENT_PRESSED 1
#define BUTTON_EVENT_RELEASED 2
#define BUTTON_EVENT_HOLD 3

typedef struct {
  uint8_t button; 
  uint8_t event;
} buttonMessage_struct;

typedef struct {
  long encoderPosition;
  uint8_t direction;
} encoderMessage_struct;

// for the ledstrip on this box
typedef struct {
  uint8_t brightness;
  uint8_t mode;
  int16_t speed;
} ledMessage_struct;

// for the leds conntected to remote box
#define LIGHT_L 1
#define LIGHT_R 2
#define LIGHT_SEAT 3
#define LIGHT_BOTTOM 4

typedef struct{
  int8_t   light;
  int8_t   effect;
  int8_t   brightness;
  int8_t   speed;
} lightsMessage_struct;

#endif