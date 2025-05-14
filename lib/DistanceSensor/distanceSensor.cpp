#include "distanceSensor.h"

#define TRIG_PIN 4
#define ECHO_PIN 2

DistanceSensor::DistanceSensor(){
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

float DistanceSensor::measure(){
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float duration_us = pulseIn(ECHO_PIN, HIGH);
    float distance_cm = 0.017 * duration_us;

    return distance_cm;
}

void DistanceSensor::measure(float* results, int count, int measurementDelay){
    for(int i=0; i<count; i++){
        results[i] = measure();
        delay(measurementDelay);
    }
}

// maybe i will implement it later
DistanceStats DistanceSensor::stats(float* results, int count){
    return DistanceStats();
}