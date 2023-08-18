// Define to prevent recursive inclusion
#ifndef BUTTONS_H
#define BUTTONS_H
#include <stdlib.h>
#include <AceButton.h>
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;

#define SWITCH1_PIN 36
#define SWITCH2_PIN 37
#define DREHSWITCH_PIN 39
#define MULTISWITCH_PIN 38
#define BUTTON_READ_PERIOD 30 // Read Buttons every 30ms

void setupButtons();
void buttontask (void* pvParameters);
void switchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);
void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);
void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);

#endif
