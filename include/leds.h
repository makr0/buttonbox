#ifndef LEDS_H
#define LEDS_H

#define LED_COUNT 8
#define LED_PIN 26

void ledtask (void* pvParameters);
void setupLeds();

#endif