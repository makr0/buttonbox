#include <SPI.h>
#include "taskmessages.h"
#include "comm.h"
#include "leds.h"
#include "display.h"
#include "buttons.h"

QueueHandle_t screenDataQueue = xQueueCreate(8, sizeof(screendata_struct));
QueueHandle_t ledbarDataQueue = xQueueCreate(8, sizeof(ledMessage_struct));
QueueHandle_t serialSendLightdataQueue = xQueueCreate(8, sizeof(lightsMessage_struct));
QueueHandle_t buttonEventsQueue = xQueueCreate(8, sizeof(buttonMessage_struct));
QueueHandle_t screenDebugQueue = xQueueCreate(8, sizeof(screenDebug_struct));

void debugToScreen(int x,int y,int fontnumber, const char *format, ...) {
  screenDebug_struct screenDebugMessage;
  va_list argptr;
  va_start(argptr, format);
  sprintf(screenDebugMessage.msg,format,argptr);
  va_end(argptr);

  screenDebugMessage.x=x;
  screenDebugMessage.y=y;
  screenDebugMessage.fontnumber=fontnumber;
  xQueueSend(screenDebugQueue, (void*)&screenDebugMessage, 0);
}

void setup(){
    setupCommports();
    setupLeds();
    setupButtons();
    setupDisplay();
//    debugToScreen(10,40,1,"Font 1");
//    debugToScreen(10,60,2,"Font 2");
//    debugToScreen(10,80,4,"Font 4");
//    debugToScreen(10,100,6,"Font 6");
//    debugToScreen(10,120,7,"Font 7"); // 7segment font
//    debugToScreen(10,200,8,"Font 8");
}

void loop() {
  delay(250);
}


