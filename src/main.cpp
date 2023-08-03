#include "FS.h"
#include <AceButton.h>
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;
#include <WS2812FX.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library

const byte ROWS = 5;
const byte COLS = 3;

#define LED_COUNT 8
#define LED_PIN 26
#define SWITCH1_PIN 36
#define SWITCH2_PIN 37
#define DREHSWITCH_PIN 39
#define MULTISWITCH_PIN 38
#define CALIBRATION_FILE "/calibrationData"
#define GRAPH_POS_X 0
#define GRAPH_POS_Y 120
#define GRAPH_W 320
#define GRAPH_H 110

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
  1400,
  2000,
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

WS2812FX ledbar = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
TFT_eSPI tft = TFT_eSPI();       // Invoke tft library
MeterWidget   amps  = MeterWidget(&tft);
//MeterWidget   volts = MeterWidget(&tft);
GraphWidget graph = GraphWidget(&tft);    // Graph widget gr instance with pointer to tft
TraceWidget trace = TraceWidget(&graph);     // Graph trace tr with pointer to gr

#define DISPLAY_LOOP_PERIOD 20 // Display update rate 50Hz
#define BUTTON_READ_PERIOD 30 // Read Buttons every 30ms

void ledtask (void* pvParameters);
void displaytask (void* pvParameters);
void buttontask (void* pvParameters);
float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax);
void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);
void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState);

void setupLeds() {
    ledbar.init();
    ledbar.setBrightness(30);
    ledbar.start();
    ledbar.setMode(FX_MODE_RAINBOW_CYCLE);
    ledbar.setSpeed(5000);
    xTaskCreatePinnedToCore (
      ledtask,     // Function to implement the task
      "led",   // Name of the task
      1000,      // Stack size in bytes
      NULL,      // Task input parameter
      0,         // Priority of the task
      NULL,      // Task handle.
      0          // Core where the task should run
    );
}

const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = 0;
const float gyHigh = 2048;

void setupDisplay() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
  // Colour zones are set as a start and end percentage of full scale (0-100)
  // If start and end of a colour zone are the same then that colour is not used
  //            --Red--  -Org-   -Yell-  -Grn-
  //amps.setZones(75, 100, 50, 75, 25, 50, 0, 25); // Example here red starts at 75% and ends at 100% of full scale
  // Meter is 239 pixels wide and 126 pixels high
  //amps.analogMeter(0, 0, 2048.0, "ADC", "0", "512", "1024", "1536", "2048");    // Draw analogue meter at 0, 0

  // Colour draw order is red, orange, yellow, green. So red can be full scale with green drawn
  // last on top to indicate a "safe" zone.
  //             -Red-   -Org-  -Yell-  -Grn-
//  volts.setZones(0, 100, 25, 75, 0, 0, 40, 60);
//  volts.analogMeter(0, 120, 10.0, "V", "0", "2.5", "5", "7.5", "10"); // Draw analogue meter at 0, 128
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
//    xTaskCreatePinnedToCore (
//      displaytask,     // Function to implement the task
//      "tft",   // Name of the task
//      2000,      // Stack size in bytes
//      NULL,      // Task input parameter
//      0,         // Priority of the task
//      NULL,      // Task handle.
//      0          // Core where the task should run
//    );
}

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
void setupTouch() {
uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  Serial.begin(115200);
  Serial.println("starting");

  tft.init();

  tft.setRotation(3);
  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
  tft.println("calibration run");

  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formating file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }
  if (calDataOK) {
    // calibration data valid
    tft.setTouch(calibrationData);
  } else {
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }

  tft.fillScreen((0xFFFF));
}

void setup(){
    Serial.begin(115200);
    setupLeds();
    setupButtons();
//    setupTouch();
    setupDisplay();
}

void ledtask (void* pvParameters) {
    while(42) {
      ledbar.service();
    }
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
void displaytask(void* pvParameters) {
  static float gx = 0.0;
  static float delta = 10.0;
  uint16_t buttonValue = 0;
  while(1) {
    static int d = 0;
    static uint32_t updateTime = 0;  

    if (millis() - updateTime >= DISPLAY_LOOP_PERIOD) 
    {
      updateTime = millis();

      float meterValue;
      buttonValue = analogRead(MULTISWITCH_PIN);
      amps.updateNeedle(buttonValue, 0);

      trace.addPoint(gx, buttonValue);
      gx += 1.0;
      // If the end of the graph x ais is reached start a new trace at 0.0,0.0
      if (gx > gxHigh) {
        gx = 0.0;
        // Draw empty graph at 40,10 on display to clear old one
        graph.drawGraph(GRAPH_POS_X, GRAPH_POS_Y);
        // Start new trace
        trace.startTrace(TFT_GREEN);
      }
    }
  }
}

void loop() {
  delay(1000);
}

void drehswitchEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  String msg;
  if(eventType == AceButton::kEventPressed) {
    msg = button->getPin();
    tft.setTextColor(TFT_DARKGREEN,TFT_BLACK);
    tft.drawCentreString(msg,290,30,4);
  }
}

struct position {
  uint16_t x;
  uint16_t y;
  uint16_t fgColor;
  uint16_t bgColor;
};

position btnIndicators[MULTISWITCH_POSITIONS] = {
  {310, 220, TFT_GREEN,TFT_BLACK},
  {240,220,TFT_GREEN,TFT_BLACK},
  {100,220,TFT_GREEN,TFT_BLACK},
  {10,220,TFT_GREEN,TFT_BLACK},
  {160,20,TFT_GREEN,TFT_BLACK}
};

void frontplateEventListener(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  
  position p;
  p = btnIndicators[button->getPin()];
  if(eventType == AceButton::kEventPressed) {
    tft.fillSmoothCircle(p.x,p.y,10,p.fgColor,p.bgColor);
  } else {
    tft.fillCircle(p.x,p.y,12,p.bgColor);
  }
}


float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax)
{
  return tomin + (((tomax - tomin) * (ip - ipmin))/ (ipmax - ipmin));
}
