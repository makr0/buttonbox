#include "OpenFontRender.h"
#include "ringmeter.h"


// #########################################################################
// Meter constructor
// #########################################################################
 RingMeterWidget::RingMeterWidget(TFT_eSPI* tft)
{
    ringthickness = 0;
    radius = 0;
    last_angle = 0;
    middle_x = 0;
    middle_y = 0;
    ringcolor = TFT_GREEN;
    ntft = tft;
    useAverager = true;
    averagerSize = 20;
    averagerIndex = 0;
    lastAverage = 0;
    currentAverage =0;
    for (uint8_t i = averagerSize; i > 0; )
    {
      valueAverager[--i] = 0.0; // keeps addValue simpler
    }
}

void RingMeterWidget::ringMeter(int x, int y, int r, int thickness, float start, float end, const char *units, int16_t color)
{
    last_angle = 45;
    ringthickness = thickness;
    middle_x=x;
    middle_y=y;
    radius = r-3;
    ringcolor = color;
    startValue=start;
    endValue=end;
    startAngle=45;
    endAngle=360-startAngle;
    meter_unit=units;
    char msg[5];
    setFormatstring( "%3.1f");

    ntft->fillCircle(x, y, r, TFT_BLACK);
    ntft->drawSmoothCircle(x, y, r, TFT_SILVER, TFT_BLACK);
    sprintf(msg,"%s",meter_unit);
    ntft->setTextColor(TFT_YELLOW,TFT_BLACK);
    ntft->drawCentreString(msg,middle_x,middle_y+7,2);
    uint16_t tmp = r - 3;
    ntft->drawArc(x, y, tmp, tmp - tmp / 5, last_angle, 330, TFT_BLACK, TFT_BLACK);
}

void RingMeterWidget::updateValue(float val) {
  if(val < startValue || val >endValue) return;

  int val_angle = map(abs(val), startValue, endValue, 45, 360-45);
  val_angle = constrain(val_angle,startAngle,endAngle);
  if(useAverager) {
    addValueToAverager(val);
    currentAverage = getAverage();
  } else {
    currentAverage = val;
  }
  if(round(currentAverage) != round(lastAverage)) {
    ntft->fillRect(middle_x-radius+ringthickness+3,middle_y-10,(radius*2)-ringthickness-15,20,TFT_BLACK);
    char msg[5];
    sprintf(msg,formatstring,currentAverage);
    ntft->setTextColor(TFT_WHITE,TFT_BLACK);
    ntft->drawCentreString(msg,middle_x,middle_y-7,2);
    lastAverage = currentAverage;
  }
  if (last_angle != val_angle) {
    // Update the arc, only the zone between last_angle and new val_angle is updated
    if (val_angle > last_angle) {
      ntft->drawArc(middle_x, middle_y, radius, radius - ringthickness, last_angle, val_angle, ringcolor, TFT_BLACK);
    }
    else {
      ntft->drawArc(middle_x, middle_y, radius, radius - ringthickness, val_angle, last_angle, TFT_BLACK, TFT_BLACK);
    }
    last_angle = val_angle; 
  }
}

void RingMeterWidget::setFormatstring(String format) {
  format.toCharArray(formatstring, sizeof(formatstring),0);
}

void RingMeterWidget::addValueToAverager(float val) {
  valueAverager[averagerIndex] = val;
  averagerIndex++;

  if (averagerIndex == averagerSize) averagerIndex = 0;  // faster than %

}
float RingMeterWidget::getAverage() {
  float _sum = 0;
  for (uint8_t i = 0; i < averagerSize; i++)
  {
    _sum += valueAverager[i];
  }
  return _sum / averagerSize;   // multiplication is faster ==> extra admin

}

void RingMeterWidget::setUseAverager(bool u){
  useAverager = u;
}
