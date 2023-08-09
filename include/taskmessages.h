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
  long encoderPosition;
  uint8_t direction;
} encoderMessage_struct;

typedef struct {
  uint8_t brightness;
  uint8_t mode;
  int16_t speed;
} ledMessage_struct;

#endif