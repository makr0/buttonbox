#include <AceButton.h>
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;
#include <SPI.h>
#include "taskmessages.h"
#include "comm.h"
#include "leds.h"
#include "display.h"

#define SWITCH1_PIN 36
#define SWITCH2_PIN 37
#define DREHSWITCH_PIN 39
#define MULTISWITCH_PIN 38

QueueHandle_t screenDataQueue = xQueueCreate(8 /* Number of queue slots */, sizeof(screendata_struct));
QueueHandle_t ledbarDataQueue = xQueueCreate(8 /* Number of queue slots */, sizeof(ledMessage_struct));
QueueHandle_t serialSendQueue = xQueueCreate(8 /* Number of queue slots */, sizeof(buttondata_struct));

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
  500,
  900,
  1700,
  2500,
  3000
};
// The LadderButtonConfig constructor binds the AceButton objects in the BUTTONS
// array to the LadderButtonConfig.
static LadderButtonConfig drehswitchButtonConfig(
  DREHSWITCH_PIN, NUM_LEVELS, LEVELS, DREHSWITCH_POSITIONS, DREHBUTTONS
);
static LadderButtonConfig frontplateButtonConfig(
  MULTISWITCH_PIN, NUM_LEVELS, LEVELS, DREHSWITCH_POSITIONS, MULTIBUTTONS
);

void buttontask (void* pvParameters);
float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax);
void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);
void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);

const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = 0;
const float gyHigh = 2048;

void setupButtons() {
  pinMode(SWITCH1_PIN,INPUT);
  pinMode(SWITCH2_PIN,INPUT);
  pinMode(DREHSWITCH_PIN,INPUT);
  pinMode(MULTISWITCH_PIN,INPUT);
  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  drehswitchButtonConfig.setEventHandler(drehswitchEventListener);
  drehswitchButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  frontplateButtonConfig.setEventHandler(frontplateEventListener);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  frontplateButtonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  
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

void setup(){
    setupCommports();
    setupLeds();
    setupButtons();
    setupDisplay();
}

void buttontask (void* pvParameters) {
    static uint32_t updateTime = 0;  
    while(42) {
      if (millis() - updateTime >= BUTTON_READ_PERIOD) 
      {
        updateTime = millis();
        drehswitchButtonConfig.checkButtons();
        frontplateButtonConfig.checkButtons();
      }
    }
}


void loop() {
  delay(1000);
}

void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  String msg;
  buttondata_struct messageToSerial;
  screendata_struct messageToScreen;
  messageToScreen.batVoltage=0;
  messageToScreen.speed=0;
  messageToScreen.boardTemp=0;
  messageToScreen.buttonState=0;
  if(eventType == AceButton::kEventPressed) {
    messageToSerial.steeringButtons=0;
    messageToSerial.modeButton = button->getPin()+1;
    xQueueSend(serialSendQueue, (void*)&messageToSerial, 0);
    messageToScreen.modeSwitch=button->getPin()+1;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
  }
}



void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  buttondata_struct messageToSerial;
  screendata_struct messageToScreen;
  messageToScreen.batVoltage=0;
  messageToScreen.speed=0;
  messageToScreen.boardTemp=0;
  messageToScreen.buttonState=0;
  if(eventType == AceButton::kEventPressed) {
    messageToSerial.steeringButtons=button->getPin()+1;
    messageToSerial.modeButton = 0;
    xQueueSend(serialSendQueue, (void*)&messageToSerial, 0);
    messageToScreen.pushButtons=button->getPin()+1;
    messageToScreen.buttonState = BUTTON_PRESS;
    messageToScreen.speed = button->getPin()+10;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);

  }
  if(eventType == AceButton::kEventReleased) {
    messageToScreen.pushButtons=button->getPin()+1;
    messageToScreen.buttonState = BUTTON_RELEASE;
    messageToScreen.batVoltage = button->getPin()+50;
    xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
  }
}


