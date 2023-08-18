#include "taskmessages.h"
#include "carConfig.h"
#include "display.h"
#include "screens/menucolors.h"
#include <menu.h>
#include <menuIO/stringIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/TFT_eSPIOut.h>
extern TFT_eSPI tft;

extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t encoderEventsQueue;;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern SemaphoreHandle_t tftSemaphore;
extern carConfig_t carConfig;

int test=55;

using namespace Menu;
#define fontW 12
#define fontH 18

#define MAX_DEPTH 2
#define textScale 2
MENU_OUTPUTS(out,MAX_DEPTH
  ,TFT_eSPI_OUT(tft,colors,6*textScale,9*textScale,{0,0,14,8},{14,0,14,8})
  ,NONE
);
stringIn<0> strIn;//buffer size: 2^5 = 32 bytes, eventually use 0 for a single byte
menuIn* inputsList[]={&strIn};
chainStream<sizeof(inputsList)> in(inputsList);

result showEvent(eventMask e) {
  Serial.print("event: ");
  Serial.println(e);
  return proceed;
}

result action1(eventMask e,navNode& nav, prompt &item) {
  Serial.print("action1 event:");
  Serial.println(e);
  Serial.flush();
  return proceed;
}

result action2(eventMask e) {
  Serial.print("actikon2 event:");
  Serial.println(e);
  Serial.flush();
  return quit;
}

SELECT(carConfig.controlMode,controlModeMenu,"Ctrl mode",doNothing,noEvent,wrapStyle
  ,VALUE("Voltage",1,doNothing,noEvent)
  ,VALUE("Speed",2,doNothing,noEvent)
  ,VALUE("Torque",3,doNothing,noEvent)
);

SELECT(carConfig.controlType,controlTypeMenu,"Ctrl type",doNothing,noEvent,wrapStyle
  ,VALUE("Commutation",0,doNothing,noEvent)
  ,VALUE("Sinus",1,doNothing,noEvent)
  ,VALUE("FOC",2,doNothing,noEvent)
);

MENU(subMenu,"Sub-Menu",doNothing,anyEvent,wrapStyle
  ,OP("Sub1",showEvent,enterEvent)
  ,OP("Sub2",showEvent,enterEvent)
  ,OP("Sub3",showEvent,enterEvent)
  ,EXIT("<Back")
);

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,OP("Op1",action1,anyEvent)
  ,OP("Op2",action2,enterEvent)
  ,SUBMENU(subMenu)
  ,SUBMENU(controlModeMenu)
  ,SUBMENU(controlTypeMenu)
);


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);
//navNode navCursors[MAX_DEPTH];//objects to control each level of navigation
//navRoot nav(mainMenu,navCursors,MAX_DEPTH-1,*(Stream*)NULL,out);

void initSetupScreen( TFT_eSPI* tft) {
    tft->setTextColor(TFT_WHITE,TFT_BLACK);
    tft->drawCentreString("Setup",DISPLAY_WIDTH/2,0,4);
    tft->drawCentreString("->",10,220,2);
    tft->drawCentreString("<-",100,220,2);
    tft->drawCentreString("ok",240,220,2);
    tft->drawCentreString("esc",310,220,2);
}
void updateSetupScreen(TFT_eSPI* tft) {
  char msg[40];
  encoderMessage_struct encoderMessage;
  buttonMessage_struct buttonMessage;
  GUARD_TFT_BEGIN
    nav.doOutput();//if not doing poll the we need to do output "manualy"
  GUARD_TFT_END
  while(1) {
    if (xQueueReceive(encoderEventsQueue, (void *)&encoderMessage, 0) == pdTRUE) {
        if(encoderMessage.direction ==1 ) {
          nav.doNav(upCmd);
        } else {
          nav.doNav(downCmd);
        }
    }
    if (xQueueReceive(buttonEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
      if(buttonMessage.event == BUTTON_PRESS) {
        if(buttonMessage.button == 3) {
          nav.doNav(rightCmd);
        }
        if(buttonMessage.button == 2) {
          nav.doNav(leftCmd);
        }
        if(buttonMessage.button == 1) {
          nav.doNav(enterCmd);
        }
        if(buttonMessage.button == 0) {
          nav.doNav(escCmd);
        }
      }
    }
    GUARD_TFT_BEGIN
      nav.doOutput();//if not doing poll the we need to do output "manualy"
    GUARD_TFT_END
    Serial.println(carConfig.controlMode);
    vTaskDelay(20/portTICK_PERIOD_MS);
  }
}


