#include <Arduino.h>

#include "distanceStats.h"

class DistanceSensor {
private:
    float SOUND_SPEED_CM_PER_US_HALF = 0.017;
public:
    DistanceSensor();

    float measure();
    void measure(float* results, int count, int measurementDelay);
    DistanceStats stats(float* results, int count);

};