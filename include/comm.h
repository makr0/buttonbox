// Define to prevent recursive inclusion
#ifndef COMM_H
#define COMM_H

#include <stdlib.h>

#define START_FRAME_CARDATA      0xABCD  // [-] Dataframe from Hoverboard
#define START_FRAME_LIGHTDATA    0xF002  // [-] Dataframe from Lightbox (contains buttons connected to Lightbox)
#define START_FRAME_BUTTONDATA   0xF003  // [-] Dataframe from Buttonbox

void serialReadFunction( void * parameter);
void setupCommports();
void serialSendLightdataFunction(void* pvParameters);

typedef struct{
  uint16_t  start;
  int16_t   cmd1;
  int16_t   cmd2;
  int16_t   speedR_meas;
  int16_t   speedL_meas;
  int16_t   batVoltage;
  int16_t   boardTemp;
  uint16_t 	cmdLed;
  uint16_t  checksum;
} HovercarDataPacket;
static HovercarDataPacket carData;

typedef struct{
  uint16_t  start;
  int16_t   nMotMax; // max rotation speed (rpm)
  int16_t   curMax; // max Current (A)
  int16_t   mode;    // driving mode (volt,torque,speed)
  uint16_t  checksum;
} HovercarControlPacket;
static HovercarControlPacket carControl;

typedef struct{
  uint16_t  start;
  int16_t   encoderPosition;
  int16_t   encoderDirection;
  int16_t   button1;
  int16_t   button2;
  uint16_t  checksum;
} LightboxDataPacket;
static LightboxDataPacket LightboxData;

#define LIGHT_L 1
#define LIGHT_R 2
#define LIGHT_SEAT 3
#define LIGHT_BOTTOM 4
#define LIGHTS_ALL 0xF

#define LIGHTCOMMAND_BRIGHTNESS 1
#define LIGHTCOMMAND_EFFECT 2
#define LIGHTCOMMAND_SPEED 3

typedef struct{
  uint16_t  start;
  int8_t   light;
  int8_t   command;
  int8_t   value;
  uint16_t  checksum;
} LightsDataPacket;
static LightsDataPacket LightsData;

#endif
