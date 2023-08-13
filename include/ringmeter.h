#ifndef ringMeter_h
#define ringMeter_h

#include "OpenFontRender.h"

#include "Arduino.h"
#include "TFT_eSPI.h"

class RingMeterWidget
{
  public:
    RingMeterWidget(TFT_eSPI* tft);
    void updateValue(float val);
    void ringMeter(int x, int y, int r, int thickness, float start, float end, const char *units, int16_t color);

  private:
    int ringthickness;
    int radius;
    int last_angle;
    // Pointer to TFT_eSPI class functions
    TFT_eSPI* ntft;
    int middle_x;
    int middle_y;
    int16_t ringcolor;
    float startValue;
    float endValue;
    int startAngle;
    int endAngle;
    const char* meter_unit;
};

#endif