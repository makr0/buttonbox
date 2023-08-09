#include <TFT_eSPI.h> // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library
#include "taskmessages.h"
#include "display.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke tft library
//MeterWidget   amps  = MeterWidget(&tft);
MeterWidget   volts = MeterWidget(&tft);
// GraphWidget graph = GraphWidget(&tft);    // Graph widget gr instance with pointer to tft
// TraceWidget trace = TraceWidget(&graph);     // Graph trace tr with pointer to gr
extern QueueHandle_t screenDataQueue;

struct position {
  uint16_t x;
  uint16_t y;
  uint16_t fgColor;
  uint16_t bgColor;
};

position btnIndicators[5] = {
  {310, 220, TFT_GREEN,TFT_BLACK},
  {240,220,TFT_RED,TFT_BLACK},
  {100,220,TFT_RED,TFT_BLACK},
  {10,220,TFT_RED,TFT_BLACK},
  {160,20,TFT_SILVER,TFT_BLACK}
};

void setupDisplay() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
  // Colour zones are set as a start and end percentage of full scale (0-100)
  // If start and end of a colour zone are the same then that colour is not used
  //            --Red--  -Org-   -Yell-  -Grn-
//  amps.setZones(75, 100, 50, 75, 25, 50, 0, 25); // Example here red starts at 75% and ends at 100% of full scale
  // Meter is 239 pixels wide and 126 pixels high
//  amps.analogMeter(0, 40, 2048.0, "ADC", "0", "512", "1024", "1536", "2048");    // Draw analogue meter at 0, 0

  // Colour draw order is red, orange, yellow, green. So red can be full scale with green drawn
  // last on top to indicate a "safe" zone.
  //             -Red-   -Org-  -Yell-  -Grn-
  volts.setZones(0, 50, 50, 75, 0, 0, 75, 100);
  volts.analogMeter(TFT_HEIGHT-239, TFT_WIDTH-126,39, 54, "V", "39", "43", "46.5", "50", "54"); // Draw analogue meter at 0, 128
//  uint16_t x = 200;
//  uint16_t y = 1;

// Graph area is 300 pixels wide, 150 pixels high, dark grey background
  //graph.createGraph(GRAPH_W, GRAPH_H, tft.color565(5, 5, 5));

  // x scale units is from 0 to 100, y scale units is -512 to 512
  //graph.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);

  // X grid starts at 0 with lines every 20 x-scale units
  // Y grid starts at -512 with lines every 64 y-scale units
  // blue grid
  //graph.setGraphGrid(gxLow, 25, gyLow, 256.0, TFT_BLUE);

  // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
  //graph.drawGraph(GRAPH_POS_X, GRAPH_POS_Y);

//  trace.startTrace(TFT_WHITE);
  xTaskCreatePinnedToCore (updateDisplayFunction,"tft",10000,NULL,0,NULL,0);
}
float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax)
{
  return tomin + (((tomax - tomin) * (ip - ipmin))/ (ipmax - ipmin));
}

void updateDisplayFunction( void * parameter) {
    char msg[40];
    screendata_struct receivedMessage;
    position p;
    while(1) {
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, portMAX_DELAY /* Wait infinitely for new messages */) == pdTRUE) {
          // show modeswitch position
          if(receivedMessage.modeSwitch != 0) {
                sprintf(msg,"%d",receivedMessage.modeSwitch);
                tft.setTextColor(TFT_GREEN,TFT_BLACK);
                tft.drawCentreString(msg,290,30,4);
          }
          // show pushbutton states
          if(receivedMessage.pushButtons != 0) {
            p = btnIndicators[receivedMessage.pushButtons-1];
            if(receivedMessage.buttonState == BUTTON_PRESS) tft.fillSmoothCircle(p.x,p.y,10,p.fgColor,p.bgColor);
            if(receivedMessage.buttonState == BUTTON_RELEASE) tft.fillCircle(p.x,p.y,12,p.bgColor);
          }
          // show speed
          if(receivedMessage.speed != 0) {
            sprintf(msg,"%d rpm",receivedMessage.speed);
            tft.setTextColor(TFT_RED,TFT_BLACK);
            tft.drawCentreString(msg,TFT_HEIGHT/2,TFT_WIDTH/2,4);
          }
          // show Voltage
          if(receivedMessage.batVoltage != 0) {
            volts.updateNeedle(receivedMessage.batVoltage,0);
          }
        }
    }
}
