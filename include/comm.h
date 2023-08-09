// Define to prevent recursive inclusion
#ifndef COMM_H
#define COMM_H

#include <stdlib.h>

#define START_FRAME_CARDATA      0xABCD  // [-] Dataframe from Hoverboard
#define START_FRAME_LIGHTDATA    0xF002  // [-] Dataframe from Lightbox (contains buttons connected to Lightbox)
#define START_FRAME_BUTTONDATA   0xF003  // [-] Dataframe from Buttonbox

void serialReadFunction( void * parameter);
void setupCommports();
void serialSendFunction(void* pvParameters);

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

typedef struct{
  uint16_t  start;
  int8_t   buttons; // 8 bits, set/not set 
  int8_t   mode; // number 1-5 corresponding to dial-button
  uint16_t  checksum;
} ButtonboxDataPacket;
static ButtonboxDataPacket ButtonData;

typedef struct {
  uint8_t steeringButtons;
  uint8_t modeButton;
} buttondata_struct;


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#endif
