#include <TFT_eSPI.h> // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library
#include "taskmessages.h"
#include "display.h"
#include "screens/mainScreen.h"
#include "screens/setupScreen.h"
#include "screens/powerScreen.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke tft library
//MeterWidget   amps  = MeterWidget(&tft);
// GraphWidget graph = GraphWidget(&tft);    // Graph widget gr instance with pointer to tft
// TraceWidget trace = TraceWidget(&graph);     // Graph trace tr with pointer to gr
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t modeSwitchEventsQueue;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern SemaphoreHandle_t tftSemaphore;
SemaphoreHandle_t tftSemaphore = NULL;

TaskHandle_t tftTask = NULL;
typedef void (* DisplayFunction_t)( TFT_eSPI* );

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

struct screenDef_t {
  uint8_t id;
  DisplayFunction_t initFN;
  DisplayFunction_t updateFN;
};

screenDef_t screens[3];
uint8_t currentScreen = 2;
uint8_t lastScreen = 2;

void registerScreen(uint8_t id,DisplayFunction_t initFN, DisplayFunction_t updateFN){
  screens[id].initFN = initFN;
  screens[id].updateFN = updateFN;
}
void setupDisplay() {
      tftSemaphore = xSemaphoreCreateMutex();
    GUARD_TFT_BEGIN
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    GUARD_TFT_END

    registerScreen(2,initMainScreen,updateMainScreen);
    registerScreen(1,initSetupScreen,updateSetupScreen);
    registerScreen(0,initPowerScreen,updatePowerScreen);
    screens[currentScreen].initFN(&tft);

    xTaskCreatePinnedToCore (updateDisplayFunction,"tft",10000,NULL,0,&tftTask,1);
    xTaskCreatePinnedToCore (manageScreensFunction,"scrmgr",10000,NULL,0,NULL,0);
}

void manageScreensFunction( void * parameter) {
  buttonMessage_struct buttonMessage;
  while(1) {
    if (xQueueReceive(modeSwitchEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
      Serial.print("ModeSwitch ");
      Serial.println(buttonMessage.button);
      // modeswitch position
      currentScreen=constrain(buttonMessage.button,0,2);
      Serial.print("new screen ");
      Serial.println(currentScreen);
      if(currentScreen!=lastScreen) {
        switchToScreen(currentScreen);
        lastScreen = currentScreen;
      }
    }
  }
}
void updateDisplayFunction( void * parameter) {
    screens[currentScreen].updateFN(&tft);
}

void switchToScreen(uint8_t num) {
    GUARD_TFT_BEGIN
    if(tftTask != NULL) {
      vTaskDelete(tftTask);
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
    tft.fillScreen(TFT_BLACK);
    screens[num].initFN(&tft);
    GUARD_TFT_END
    xTaskCreatePinnedToCore (updateDisplayFunction,"tft",10000,NULL,0,&tftTask,1);
}