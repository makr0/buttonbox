#include "ringmeter.h"
#include "taskmessages.h"
#include "display.h"


extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t encoderEventsQueue;
extern QueueHandle_t switchEventsQueue;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledbarDataQueue;
extern SemaphoreHandle_t tftSemaphore;
extern QueueHandle_t serialSendLightdataQueue;


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
    char msg[40];
    voltsWidget.ringMeter(ringmeters_left+15, DISPLAY_HEIGHT-50, 50, 10, 39, 54.6, "V", TFT_GREEN);
    voltsWidget.setUseAverager(false);
    wattMeter.ringMeter(ringmeters_left, DISPLAY_HEIGHT-102-35, 35, 8, 0, 55*50, "W", TFT_YELLOW);
    wattMeter.setFormatstring("%3.0f");
    ledWidget.ringMeter(ringmeters_left,35, 35, 8, 0, 255, "brgt", TFT_BLUE);
    ledWidget.setFormatstring("%3.0f");
    ledWidget.setUseAverager(false);
    tft->setTextColor(TFT_YELLOW,TFT_BLACK);
  
    tft->drawString("km/h",SPEEDMETER_X+5,SPEEDMETER_Y-20,2);
    tft->drawRightString("Lights",DISPLAY_WIDTH,30,2);
    tft->setTextColor(TFT_WHITE,TFT_BLACK);
    sprintf(msg,"max: %d W     ",powerStats.maxPower);
    tft->drawString(msg,110,DISPLAY_HEIGHT-20,2);

    screendata_struct messageToScreen;
    messageToScreen.speed = 0;
    messageToScreen.batVoltage = 0;
    messageToScreen.boardTemp = 0;
    messageToScreen.revolutions_l=0;
    messageToScreen.revolutions_r=0;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
}
void updateMainScreen(TFT_eSPI* ntft) {
    char msg[40];
    screendata_struct receivedMessage;
    screendata_struct lastMessage;
    ledMessage_struct ledMessage;
    encoderMessage_struct encoderMessage;
    buttonMessage_struct buttonMessage;
    screenDebug_struct debugMessage;
    ledMessage_struct messageToLED;
    lightsMessage_struct messageToLightbox;

    int last_angle = 45;
    int last_angle_cmd = 45;
    uint8_t lightsOn = 0;
    uint8_t lightsBrightness = 20;
    uint8_t updateLights = 0;
    unsigned long start = 0;
    while(1) {
        start = millis();
        if (xQueueReceive(screenDebugQueue, (void *)&debugMessage, 0) == pdTRUE) {
            GUARD_TFT_BEGIN
            ntft->setTextColor(TFT_RED,TFT_BLACK);
            ntft->drawString(debugMessage.msg,debugMessage.x,debugMessage.y,debugMessage.fontnumber);
            GUARD_TFT_END
        }
        if (xQueueReceive(encoderEventsQueue, (void *)&encoderMessage, 0) == pdTRUE) {
          if(lightsOn)  {
            lightsBrightness=encoderMessage.encoderPosition;
          }
          updateLights=1;
        }

        if (xQueueReceive(buttonEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
            if(buttonMessage.event == BUTTON_PRESS && buttonMessage.button == 4) {
              ESP.restart();
            }
            if(buttonMessage.event == BUTTON_RELEASE && buttonMessage.button == 2) {
              powerStats.maxPower=0;
              GUARD_TFT_BEGIN
              ntft->setTextColor(TFT_WHITE,TFT_BLACK);
              ntft->drawString("max: 0 W    ",110,DISPLAY_HEIGHT-20,2);
              GUARD_TFT_END
            }
        }
        if (xQueueReceive(switchEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
            if(buttonMessage.button == 1) {
              updateLights=1;
              if(buttonMessage.event == BUTTON_RELEASE) {
                  sprintf(msg,"off");
                  lightsOn=0;
              }
              if(buttonMessage.event == BUTTON_PRESS) {
                  sprintf(msg,"on");
                  lightsOn=1;
              }
              GUARD_TFT_BEGIN
              ntft->setTextColor(TFT_WHITE,TFT_BLACK);
              ntft->drawRightString(msg,DISPLAY_WIDTH,50,4);
              GUARD_TFT_END
            }
        }
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
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
        if(powerStats.power > powerStats.maxPower ) {
            powerStats.maxPower = powerStats.power; 
            sprintf(msg,"max: %d W     ",powerStats.maxPower);
            GUARD_TFT_BEGIN
            ntft->setTextColor(TFT_WHITE,TFT_BLACK);
            ntft->drawString(msg,110,DISPLAY_HEIGHT-20,2);
            GUARD_TFT_END
        }
        if(updateLights) {
            updateLights=0;
            if(lightsOn) {
              messageToLED.brightness = lightsBrightness;
            } else {
              messageToLED.brightness = 0;
            }
            xQueueSend(ledbarDataQueue, (void*)&messageToLED, 0);
            GUARD_TFT_BEGIN
              ledWidget.updateValue(messageToLED.brightness);
            GUARD_TFT_END
            messageToLightbox.command = LIGHTCOMMAND_BRIGHTNESS;
            messageToLightbox.value=messageToLED.brightness;
            messageToLightbox.light = LIGHT_L;
            xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
            messageToLightbox.light = LIGHT_R;
            xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
            messageToLightbox.light = LIGHT_SEAT;
            xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
        }

    }
}

