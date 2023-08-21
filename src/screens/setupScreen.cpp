#include "taskmessages.h"
#include "carConfig.h"
#include "display.h"
#include "screens/menucolors.h"
#include <streamFlow.h>
#include <menu.h>
#include <menuIO/stringIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/TFT_eSPIOut.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

extern TFT_eSPI tft;

extern QueueHandle_t screenDataQueue;
extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t switchEventsQueue;
extern QueueHandle_t encoderEventsQueue;
extern QueueHandle_t screenDebugQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern SemaphoreHandle_t tftSemaphore;
extern carConfig_t carConfig;

using namespace Menu;

#define MAX_DEPTH 5
#define textScale 2
Menu::idx_t outTops[MAX_DEPTH] = {0};
panel outPanels[] ={{1,1,25,10}};
Menu::navNode* outPanelsNode[sizeof(outPanels)/sizeof(panel)];
Menu::panelsList outPanelsList(outPanels,outPanelsNode,sizeof(outPanels)/sizeof(panel));
Menu::TFT_eSPIOut tftOut(tft,colors,outTops,outPanelsList,3*textScale,9*textScale);
Menu::serialOut serialO(Serial,outTops);
Menu::menuOut* out_outPtrs[] = { &tftOut, &serialO };
Menu::outputsList out(out_outPtrs,sizeof(out_outPtrs)/sizeof(Menu::menuOut*));;
//stringIn<0> strIn;//buffer size: 2^5 = 32 bytes, eventually use 0 for a single byte
//serialIn serial(Serial);
//menuIn* inputsList[]={&strIn,&serial};
//chainStream<sizeof(inputsList)> in(inputsList);
menuIn* inputsList[]={};
chainStream<sizeof(inputsList)> in(inputsList);

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

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,SUBMENU(controlModeMenu)
  ,SUBMENU(controlTypeMenu)
  ,FIELD(carConfig.fieldWeakEn,"FW enable","",0,1,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(carConfig.fieldWeakLo,"Fw Lo","",0,1500,50,0,doNothing,noEvent,wrapStyle)
  ,FIELD(carConfig.fieldWeakHi,"Fw Hi","",0,1500,50,0,doNothing,noEvent,wrapStyle)
  ,FIELD(carConfig.fieldWeakMax,"Fw Max"," A",0,20,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(carConfig.phaAdvMax,"phaAdvMax"," deg",0,55,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(carConfig.maxRPM,"RPM"," max",0,1500,10,0,doNothing,noEvent,wrapStyle)
);


NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void initSetupScreen( TFT_eSPI* tft) {
    tft->setTextColor(TFT_WHITE,TFT_BLACK);
    tft->drawCentreString("ok",10,220,2);
    tft->drawCentreString("^↑⇧⏫",100,220,2);
    tft->drawCentreString("V↓⇧⏬",240,220,2);
    tft->drawCentreString("esc",310,220,2);
    tft->setTextFont(2);
    nav.refresh();
    nav.doOutput();
}
void updateSetupScreen(TFT_eSPI* tft) {
  char msg[40];
  uint8_t updateNavOutput=0;
  encoderMessage_struct encoderMessage;
  buttonMessage_struct buttonMessage;
  GUARD_TFT_BEGIN
    tft->setTextFont(2);

    nav.doOutput();//if not doing poll the we need to do output "manualy"
  GUARD_TFT_END
  while(1) {
    if (xQueueReceive(encoderEventsQueue, (void *)&encoderMessage, 0) == pdTRUE) {
        if(encoderMessage.direction ==1 ) {
          nav.doNav(upCmd);
        } else {
          nav.doNav(downCmd);
        }
        updateNavOutput=1;
    }
    if (xQueueReceive(switchEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
      if(buttonMessage.button == 2) {
          if(buttonMessage.event == BUTTON_RELEASE) {
              carConfig.fieldWeakEn=0;
          }
          if(buttonMessage.event == BUTTON_PRESS) {
              carConfig.fieldWeakEn=1;
          }
          updateNavOutput=1;
      }
  }

    if (xQueueReceive(buttonEventsQueue, (void *)&buttonMessage, 0) == pdTRUE) {
      if(buttonMessage.event == BUTTON_PRESS) {
        if(buttonMessage.button == 0) {
          nav.doNav(escCmd);
        }
        if(buttonMessage.button == 1) {
          nav.doNav(upCmd);
        }
        if(buttonMessage.button == 2) {
          nav.doNav(downCmd);
        }
        if(buttonMessage.button == 3) {
          nav.doNav(enterCmd);
        }
        updateNavOutput=1;
      }
    }
    if(updateNavOutput) {
        GUARD_TFT_BEGIN
        nav.doOutput();
        GUARD_TFT_END
        updateNavOutput=0;
    }
    vTaskDelay(20/portTICK_PERIOD_MS);
  }
}


