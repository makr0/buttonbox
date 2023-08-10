#include <TFT_eSPI.h> // Hardware-specific library
#ifndef DISPLAY_H
#define DISPLAY_H

void updateDisplayFunction( void * parameter);
void setupDisplay();

// display is rotated 90° degrees
#define DISPLAY_HEIGHT TFT_WIDTH
#define DISPLAY_WIDTH TFT_HEIGHT

#endif
