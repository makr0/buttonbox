#include <TFT_eSPI.h> // Hardware-specific library
#ifndef DISPLAY_H
#define DISPLAY_H

void updateDisplayFunction( void * parameter);
void setupDisplay();

#define GRAPH_POS_X 0
#define GRAPH_POS_Y 120
#define GRAPH_W 320
#define GRAPH_H 110
#define DISPLAY_LOOP_PERIOD 20 // Display update rate 50Hz
#define BUTTON_READ_PERIOD 30 // Read Buttons every 30ms

#endif
