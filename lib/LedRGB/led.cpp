#include "led.h"

RGBLed::RGBLed(){
    colors[STARTING] = Color(255, 255, 1);
    colors[PROCESSING] = Color(1, 1, 255);
    colors[ALLOWED] = Color(1, 255, 1);
    colors[DENIED] = Color(255, 1, 1);

    pinMode(PIN_RED,   OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE,  OUTPUT);
}

void RGBLed::setLedColor(int R, int G, int B, LedMode mode){
    Color c = Color(R, G, B);
    colors[mode] = c;
}

void RGBLed::changeColor(LedMode mode){
    Color c = colors[mode];
    analogWrite(PIN_RED,   c.R);
    analogWrite(PIN_GREEN, c.G);
    analogWrite(PIN_BLUE,  c.B);
}