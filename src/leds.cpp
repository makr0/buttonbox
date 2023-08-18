#include <WS2812FX.h>
#include "taskmessages.h"
#include "leds.h"

WS2812FX ledbar = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
extern QueueHandle_t ledbarDataQueue;

void setupLeds() {
    ledbar.init();
    ledbar.setBrightness(10);
    ledbar.start();
    ledbar.setMode(FX_MODE_RAINBOW_CYCLE);
    ledbar.setSpeed(2000);
    xTaskCreatePinnedToCore ( ledtask,"led",2000,NULL,0,NULL,0 );
}

void ledtask (void* pvParameters) {
    ledMessage_struct receivedMessage;
    unsigned long lastShow = 0;
    lastShow = millis();
    uint8_t ledsTolightup = 0;

    while(1) {
      if (xQueueReceive(ledbarDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
        if(receivedMessage.brightness >= 128) {ledsTolightup=LED_COUNT;}
        else {ledsTolightup = map(receivedMessage.brightness,0,128,0,LED_COUNT);}
        ledbar.pause();
        ledbar.fill(ORANGE,0,LED_COUNT);
        if(LED_COUNT-ledsTolightup > 0) ledbar.fill(BLACK,0,LED_COUNT-ledsTolightup);
        ledbar.show();
        lastShow=millis();
//        if(receivedMessage.speed != 0) ledbar.setSpeed(receivedMessage.speed);
//        if(receivedMessage.mode  != 0 ) ledbar.setMode(receivedMessage.mode);
//        if(receivedMessage.brightness != 0 ) ledbar.setBrightness(receivedMessage.brightness);
      }
      if(millis()-lastShow>2000) {
        ledbar.resume();
      }
      ledbar.service();
    }
}
