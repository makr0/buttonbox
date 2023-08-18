#include "ringmeter.h"
#include "taskmessages.h"
#include "display.h"


extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern SemaphoreHandle_t tftSemaphore;


extern TFT_eSPI tft;
RingMeterWidget voltsWidget = RingMeterWidget(&tft);
RingMeterWidget wattMeter = RingMeterWidget(&tft);
RingMeterWidget ledWidget = RingMeterWidget(&tft);
struct powerStats_struct {
  uint16_t power;
  uint16_t maxPower;
};
powerStats_struct powerStats;
#define SPEEDMETER_X (DISPLAY_WIDTH/2+20)
#define SPEEDMETER_Y (DISPLAY_HEIGHT/2-22)

// TODO: max current = 50 konfigurierbar machen
intptr_t drawPowerArc(float val,int last_angle,TFT_eSPI* tft) {
  int val_angle = map(val, 0, 50, 45, 360-60);
  int middle_x=SPEEDMETER_X;
  int middle_y=SPEEDMETER_Y;
  int radius = 98;
  int ringthickness=6;
  int ringcolor=TFT_GREEN;

  if (last_angle != val_angle) {
    // Update the arc, only the zone between last_angle and new val_angle is updated
    if (val_angle > last_angle) {
      tft->drawArc(middle_x, middle_y, radius, radius - ringthickness, last_angle, val_angle, ringcolor, TFT_BLACK);
    }
    else {
      tft->drawArc(middle_x, middle_y, radius, radius - ringthickness, val_angle, last_angle, TFT_BLACK, TFT_BLACK);
    }
  }
  return val_angle; 
}
#define MAX_SPEED 40
intptr_t drawSpeedArc(float val,int last_angle,TFT_eSPI* tft) {
  int val_angle = map(val, 0, MAX_SPEED, 45, 360-60);
  int middle_x=SPEEDMETER_X;
  int middle_y=SPEEDMETER_Y;
  int radius = 90;
  int ringthickness=30;
  int ringcolor=TFT_GREENYELLOW;

  if (last_angle != val_angle) {
    // Update the arc, only the zone between last_angle and new val_angle is updated
    if (val_angle > last_angle) {
      tft->drawArc(middle_x, middle_y, radius, radius - ringthickness, last_angle, val_angle, ringcolor, TFT_BLACK);
    }
    else {
      tft->drawArc(middle_x, middle_y, radius, radius - ringthickness, val_angle, last_angle, TFT_BLACK, TFT_BLACK);
    }
  }
  return val_angle; 
}

void initMainScreen( TFT_eSPI* tft) {
    #define ringmeters_left 40
    voltsWidget.ringMeter(ringmeters_left+15, DISPLAY_HEIGHT-50, 50, 10, 39, 54.6, "V", TFT_GREEN);
    voltsWidget.setUseAverager(false);
    wattMeter.ringMeter(ringmeters_left, DISPLAY_HEIGHT-102-35, 35, 8, 0, 55*50, "W", TFT_YELLOW);
    wattMeter.setFormatstring("%3.0f");
    ledWidget.ringMeter(ringmeters_left,35, 35, 8, 0, 255, "brgt", TFT_BLUE);
    ledWidget.setFormatstring("%3.0f");
    tft->setTextColor(TFT_YELLOW,TFT_BLACK);
  
    tft->drawString("km/h",SPEEDMETER_X+5,SPEEDMETER_Y-20,2);
    tft->drawRightString("Lights",DISPLAY_WIDTH,30,2);
    screendata_struct messageToScreen;
    messageToScreen.speed = 0;
    messageToScreen.batVoltage = 0;
    messageToScreen.boardTemp = 0;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
}
void updateMainScreen(TFT_eSPI* ntft) {
    char msg[40];
    screendata_struct receivedMessage;
    screendata_struct lastMessage;
    ledMessage_struct ledMessage;
    buttonMessage_struct buttonMessage;
    screenDebug_struct debugMessage;
    int last_angle = 45;
    int last_angle_cmd = 45;
    unsigned long start = 0;
    while(1) {
        start = millis();
        if (xQueueReceive(screenDebugQueue, (void *)&debugMessage, 0) == pdTRUE) {
            GUARD_TFT_BEGIN
            ntft->setTextColor(TFT_RED,TFT_BLACK);
            ntft->drawString(debugMessage.msg,debugMessage.x,debugMessage.y,debugMessage.fontnumber);
            GUARD_TFT_END
        }
        if (xQueueReceive(buttonEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
            if(buttonMessage.event == BUTTON_PRESS && buttonMessage.button == 4) {
                ESP.restart();
            }
            if(buttonMessage.event == BUTTON_RELEASE && buttonMessage.button == 2) {
                powerStats.maxPower=0;
            }
        }
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
            // TODO: make switchbuttonQueue
            if(receivedMessage.switchButton == 1) {
                if(receivedMessage.switchEvent == BUTTON_RELEASE) {
                    sprintf(msg,"off");
                }
                if(receivedMessage.switchEvent == BUTTON_PRESS) {
                    sprintf(msg,"on");
                }
                GUARD_TFT_BEGIN
                ntft->drawRightString(msg,DISPLAY_WIDTH,50,4);
                GUARD_TFT_END

            } else {
            // show speed
            sprintf(msg," %d",(int)abs(receivedMessage.speed));
            GUARD_TFT_BEGIN
            ntft->setTextColor(TFT_WHITE,TFT_BLACK);
            ntft->drawRightString(msg,SPEEDMETER_X+35,SPEEDMETER_Y,7);
            last_angle = drawSpeedArc(abs(receivedMessage.speed),last_angle,ntft);
            // show power (A)
            last_angle_cmd = drawPowerArc(abs(receivedMessage.currentDC),last_angle_cmd,ntft);
            // show Voltage
            voltsWidget.updateValue(receivedMessage.batVoltage);
            powerStats.power = abs(receivedMessage.currentDC) * receivedMessage.batVoltage;
            if(powerStats.power > powerStats.maxPower ) {
                powerStats.maxPower = powerStats.power; 
                sprintf(msg,"max: %d W     ",powerStats.maxPower);
                ntft->drawString(msg,110,DISPLAY_HEIGHT-20,2);
            }
            wattMeter.updateValue(powerStats.power);
            if(receivedMessage.drivingBackwards != lastMessage.drivingBackwards) {
                if(receivedMessage.drivingBackwards == 1) {
                sprintf(msg,"rev");
                } else {
                sprintf(msg," fwd");
                }
                ntft->drawRightString(msg,100,2,2);
            }
            lastMessage = receivedMessage;
            sprintf(msg,"l: %d",receivedMessage.revolutions_l);
            ntft->drawRightString(msg,DISPLAY_WIDTH,DISPLAY_HEIGHT-20,2);
            sprintf(msg,"r: %d",receivedMessage.revolutions_r);
            ntft->drawRightString(msg,DISPLAY_WIDTH,DISPLAY_HEIGHT-40,2);
            GUARD_TFT_END
            }
        }
        if (xQueueReceive(ledMessageToScreenQueue, (void *)&ledMessage, 0) == pdTRUE) {
            GUARD_TFT_BEGIN
            ledWidget.updateValue(ledMessage.brightness);
            GUARD_TFT_END
        }
    }
}

