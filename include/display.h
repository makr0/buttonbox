#include <TFT_eSPI.h> // Hardware-specific library
#ifndef DISPLAY_H
#define DISPLAY_H

void updateDisplayFunction( void * parameter);
void setupDisplay();
void registerScreen(uint8_t id,TaskFunction_t initFN, TaskFunction_t updateFN);
void manageScreensFunction( void * parameter);
void switchToScreen(uint8_t num);


// display is rotated 90° degrees
#define DISPLAY_HEIGHT TFT_WIDTH
#define DISPLAY_WIDTH TFT_HEIGHT

#endif
