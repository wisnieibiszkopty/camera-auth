#define PIN_RED 13
#define PIN_GREEN 14
#define PIN_BLUE 15

#include <Arduino.h>
#include <map>

#include "ledMode.h"
#include "color.h"

class RGBLed {
public:
    RGBLed();

    void setLedColor(int R, int G, int B, LedMode mode);
    void changeColor(LedMode mode);
private:
    std::map<LedMode, Color> colors;
};