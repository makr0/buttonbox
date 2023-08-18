#include <AceButton.h>
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;
#include "taskmessages.h"
#include "buttons.h"

extern QueueHandle_t buttonEventsQueue;
extern QueueHandle_t modeSwitchEventsQueue;
extern QueueHandle_t serialSendLightdataQueue;
extern QueueHandle_t ledbarDataQueue;
extern QueueHandle_t screenDataQueue;

static AceButton switch1(SWITCH1_PIN);
static AceButton switch2(SWITCH2_PIN);

static const uint8_t DREHSWITCH_POSITIONS = 5;
static AceButton d0(nullptr, 0);
static AceButton d1(nullptr, 1);
static AceButton d2(nullptr, 2);
static AceButton d3(nullptr, 3);
static AceButton d4(nullptr, 4);
static AceButton m0(nullptr, 0);
static AceButton m1(nullptr, 1);
static AceButton m2(nullptr, 2);
static AceButton m3(nullptr, 3);
static AceButton m4(nullptr, 4);
static AceButton* const DREHBUTTONS[DREHSWITCH_POSITIONS] = {
    &d0, &d1, &d2, &d3, &d4
};
static const uint8_t MULTISWITCH_POSITIONS = 5;
static AceButton* const MULTIBUTTONS[MULTISWITCH_POSITIONS] = {
    &m0, &m1, &m2, &m3, &m4
};
// Define the ADC voltage levels for each button
static const uint8_t NUM_LEVELS = DREHSWITCH_POSITIONS + 1;
static const uint16_t LEVELS[NUM_LEVELS] = {
  0,    
  670,   // 670
  1290,  // 1290
  2115,  // 2115
  2800,  // 2800
  4096   // 
};
static ButtonConfig switchButtonConfig();
// The LadderButtonConfig constructor binds the AceButton objects in the BUTTONS
// array to the LadderButtonConfig.
static LadderButtonConfig drehswitchButtonConfig(
  DREHSWITCH_PIN, NUM_LEVELS, LEVELS, DREHSWITCH_POSITIONS, DREHBUTTONS
);
static LadderButtonConfig frontplateButtonConfig(
  MULTISWITCH_PIN, NUM_LEVELS, LEVELS, DREHSWITCH_POSITIONS, MULTIBUTTONS
);


void setupButtons() {
  pinMode(SWITCH1_PIN,INPUT);
  pinMode(SWITCH2_PIN,INPUT);
  pinMode(DREHSWITCH_PIN,INPUT);
  pinMode(MULTISWITCH_PIN,INPUT);
  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* switch1Config = switch1.getButtonConfig();
  ButtonConfig* switch2Config = switch2.getButtonConfig();
  switch1Config->setEventHandler(switchEventListener);
  switch2Config->setEventHandler(switchEventListener);
  drehswitchButtonConfig.setEventHandler(drehswitchEventListener);
  drehswitchButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  frontplateButtonConfig.setEventHandler(frontplateEventListener);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  frontplateButtonConfig.setDebounceDelay(80);
  
    xTaskCreatePinnedToCore (
      buttontask,     // Function to implement the task
      "btn",   // Name of the task
      5000,      // Stack size in bytes
      NULL,      // Task input parameter
      0,         // Priority of the task
      NULL,      // Task handle.
      0          // Core where the task should run
    );
}

void buttontask (void* pvParameters) {
    static uint32_t updateTime = 0;  
    while(42) {
      if (millis() - updateTime >= BUTTON_READ_PERIOD) 
      {
        updateTime = millis();
        drehswitchButtonConfig.checkButtons();
        frontplateButtonConfig.checkButtons();
        switch1.check();
        switch2.check();
      }
    }
}

void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  buttonMessage_struct buttonMessage;
  screendata_struct messageToScreen;
  if(eventType == AceButton::kEventPressed) {
    buttonMessage.button=button->getPin();
    buttonMessage.event=BUTTON_PRESS;
    xQueueSend(modeSwitchEventsQueue, (void*)&buttonMessage, 0);
  }
}

void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  buttonMessage_struct buttonMessage;
  screendata_struct messageToScreen;
  buttonMessage.button=button->getPin();
  if(eventType == AceButton::kEventPressed) {
    buttonMessage.event=BUTTON_PRESS;
    xQueueSend(buttonEventsQueue, (void*)&buttonMessage, 0);
  }
  if(eventType == AceButton::kEventReleased) {
    buttonMessage.event=BUTTON_RELEASE;
    xQueueSend(buttonEventsQueue, (void*)&buttonMessage, 0);
  }
}

void switchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  ledMessage_struct messageToLED;
  ledMessage_struct ledMessageToScreen;
  screendata_struct messageToScreen;
  lightsMessage_struct messageToLightbox;

  Serial.printf("button %d:%d (%d)\n", button->getPin(), eventType, buttonState);
  messageToScreen.switchEvent = eventType == AceButton::kEventReleased ? BUTTON_RELEASE : BUTTON_PRESS;
  messageToScreen.switchButton = button->getPin() == SWITCH1_PIN ? 1 : 2;
  xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
  switch (button->getPin()) {
    case SWITCH1_PIN: 
      if(eventType == AceButton::kEventReleased) {
        messageToLED.brightness=0;
        messageToLightbox.value=0;
      }
      if(eventType == AceButton::kEventPressed) {
        messageToLED.brightness=20;
        messageToLightbox.value=20;
      }

      xQueueSend(ledbarDataQueue, (void*)&messageToLED, 0);
      messageToLightbox.command = LIGHTCOMMAND_BRIGHTNESS;
      messageToLightbox.light = LIGHT_L;
      xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
      messageToLightbox.light = LIGHT_R;
      xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
      messageToLightbox.light = LIGHT_SEAT;
      xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
      break;
    default:
      break;
  }

}