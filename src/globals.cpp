#include <Arduino.h>
#include "globals.h"

// for basic esp
//const gpio_num_t WAKEUP_GPIO = GPIO_NUM_15;

// for esp with camera
const gpio_num_t WAKEUP_GPIO = GPIO_NUM_32;

const int TRIG_PIN = 4;
const int ECHO_PIN = 2;