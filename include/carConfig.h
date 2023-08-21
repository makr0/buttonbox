#include <stdlib.h>

#ifndef CARCONFIG_H
#define CARCONFIG_H

typedef struct {
  uint8_t  controlMode;
  uint8_t  controlType;
  uint16_t maxRPM;
  uint8_t  fieldWeakEn;
  int16_t  fieldWeakHi;
  int16_t  fieldWeakLo;
  int16_t  fieldWeakMax;
  int16_t  phaAdvMax;
} carConfig_t;


#endif