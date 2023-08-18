#include "taskmessages.h"
#include "display.h"


extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern SemaphoreHandle_t tftSemaphore;


void initPowerScreen( TFT_eSPI* tft) {
    tft->setTextColor(TFT_WHITE,TFT_BLACK);
    tft->drawCentreString("Power",DISPLAY_WIDTH/2,0,4);
}
void updatePowerScreen(TFT_eSPI* ntft) {
    char msg[40];
    while(1) {
      sprintf(msg,"%d",millis());
      GUARD_TFT_BEGIN
      ntft->drawString(msg,0,50,4);
      GUARD_TFT_END
      vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

