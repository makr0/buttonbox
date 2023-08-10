#include <TFT_eSPI.h> // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library
#include "taskmessages.h"
#include "display.h"
#include "ringmeter.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke tft library
//MeterWidget   amps  = MeterWidget(&tft);
RingMeterWidget   volts = RingMeterWidget(&tft);
// GraphWidget graph = GraphWidget(&tft);    // Graph widget gr instance with pointer to tft
// TraceWidget trace = TraceWidget(&graph);     // Graph trace tr with pointer to gr
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t screenDebugQueue;

struct position {
  uint16_t x;
  uint16_t y;
  uint16_t fgColor;
  uint16_t bgColor;
};

position btnIndicators[5] = {
  {310, 220, TFT_GREEN,TFT_BLACK},
  {240,220,TFT_RED,TFT_BLACK},
  {100,220,TFT_RED,TFT_BLACK},
  {10,220,TFT_RED,TFT_BLACK},
  {160,20,TFT_SILVER,TFT_BLACK}
};

void setupDisplay() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    volts.ringMeter(TFT_HEIGHT-40, TFT_WIDTH-40, 40, 8, 39, 54.6, "V", TFT_GREEN);
    tft.setTextColor(TFT_YELLOW,TFT_BLACK);
    tft.drawString("km/h",DISPLAY_WIDTH/2+5,DISPLAY_HEIGHT/2+34,2);

    screendata_struct messageToScreen;
    messageToScreen.speed = 0;
    messageToScreen.batVoltage = 0;
    messageToScreen.boardTemp = 0;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
    xTaskCreatePinnedToCore (updateDisplayFunction,"tft",10000,NULL,0,NULL,0);
}
#define MAX_SPEED 35

intptr_t drawSpeedArc(int val,int last_angle) {
  int val_angle = map(val, 0, MAX_SPEED, 45, 360-45);
  int middle_x=DISPLAY_WIDTH/2-30;
  int middle_y=DISPLAY_HEIGHT/2;
  int radius = 120;
  int ringthickness=30;
  int ringcolor=TFT_GREENYELLOW;

  if (last_angle != val_angle) {
    // Update the arc, only the zone between last_angle and new val_angle is updated
    if (val_angle > last_angle) {
      tft.drawArc(middle_x, middle_y, radius, radius - ringthickness, last_angle, val_angle, ringcolor, TFT_BLACK);
    }
    else {
      tft.drawArc(middle_x, middle_y, radius, radius - ringthickness, val_angle, last_angle, TFT_BLACK, TFT_BLACK);
    }
  }
  return val_angle; 
}

void updateDisplayFunction( void * parameter) {
    char msg[40];
    screendata_struct receivedMessage;
    buttonMessage_struct buttonMessage;
    screenDebug_struct debugMessage;
    position p;
    int last_angle;
    while(1) {
        if (xQueueReceive(screenDebugQueue, (void *)&debugMessage, 0) == pdTRUE) {
            tft.setTextColor(TFT_RED,TFT_BLACK);
            tft.drawString(debugMessage.msg,debugMessage.x,debugMessage.y,debugMessage.fontnumber);
        }
        if (xQueueReceive(buttonEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
          // show modeswitch position
          if(buttonMessage.button >=10 ) {
                sprintf(msg,"%d",buttonMessage.button);
                tft.setTextColor(TFT_GREEN,TFT_BLACK);
                tft.drawCentreString(msg,290,30,4);
          } else {
            // show pushbutton states
              p = btnIndicators[buttonMessage.button];
              if(buttonMessage.event == BUTTON_PRESS) tft.fillSmoothCircle(p.x,p.y,10,p.fgColor,p.bgColor);
              if(buttonMessage.event == BUTTON_RELEASE) tft.fillCircle(p.x,p.y,12,p.bgColor);
          }
        }
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
          // show speed
            sprintf(msg,"  %d",receivedMessage.speed);
            tft.setTextColor(TFT_WHITE,TFT_BLACK);
            tft.drawRightString(msg,DISPLAY_WIDTH/2,DISPLAY_HEIGHT/2,7);
            last_angle = drawSpeedArc(receivedMessage.speed,last_angle);
          }
          // show Voltage
          if(receivedMessage.batVoltage != 0) {
            volts.updateValue(receivedMessage.batVoltage);
          }
    }
}
