#include <WS2812FX.h>
#include "taskmessages.h"
#include "leds.h"

WS2812FX ledbar = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
extern QueueHandle_t ledbarDataQueue;

void setupLeds() {
    ledbar.init();
    ledbar.setBrightness(30);
    ledbar.start();
    ledbar.setMode(FX_MODE_RAINBOW_CYCLE);
    ledbar.setSpeed(2000);
    xTaskCreatePinnedToCore ( ledtask,"led",2000,NULL,0,NULL,0 );
}

void ledtask (void* pvParameters) {
    ledMessage_struct receivedMessage;
    while(1) {
      if (xQueueReceive(ledbarDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
        if(receivedMessage.speed != 0) ledbar.setSpeed(receivedMessage.speed);
        if(receivedMessage.mode  != 0 ) ledbar.setMode(receivedMessage.mode);
        if(receivedMessage.brightness != 0 ) ledbar.setBrightness(receivedMessage.brightness);
      }
      ledbar.service();
    }
}
