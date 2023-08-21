#ifndef ringMeter_h
#define ringMeter_h

#include "Arduino.h"
#include "TFT_eSPI.h"

class RingMeterWidget
{
  public:
    RingMeterWidget(TFT_eSPI* tft);
    void updateValue(float val);
    void ringMeter(int x, int y, int r, int thickness, float start, float end, const char *units, int16_t color);
    void setFormatstring(String format);
    void setUseAverager(bool u);


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
    bool useAverager;
    const char* meter_unit;
    char formatstring[6];
    float valueAverager[20];
    uint8_t averagerSize;
    uint8_t averagerIndex;
    float lastAverage;
    float currentAverage;
    void addValueToAverager(float val);
    float getAverage();
};

#endif