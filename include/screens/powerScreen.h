#include <TFT_eSPI.h> // Hardware-specific library
#ifndef POWERSCREEN_H
#define POWERSCREEN_H

void initPowerScreen( TFT_eSPI* tft);
void updatePowerScreen(TFT_eSPI* tft);

#endif