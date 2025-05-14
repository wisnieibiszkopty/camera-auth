#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "distanceSensor.h"

#include "structs.h"
#include "globals.h"
#include "secrets.h"

const char* ssid = "Forsen";
const char* password = "forsenforsen";

void connectToWifi(){
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi!");
}

void handleWakeup(){
  auto sensor = DistanceSensor();

  int count = 5;
  float results[count];

  sensor.measure(results, count, 500);

  for (int i = 0; i < 5; i++) {
    Serial.println(results[i]);
  }

}

void setup() {
  Serial.begin(115200);

  delay(1000);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if(cause == ESP_SLEEP_WAKEUP_EXT0){
    Serial.println("Obudziles bestie!!!!");
    // connectToWifi();
    handleWakeup();
  } else {
    Serial.println("doesn't work");
  }

  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1);
  pinMode(WAKEUP_GPIO, INPUT_PULLUP);

  Serial.println("Going to sleep...");
  delay(500);
  esp_deep_sleep_start();

}

void loop() {
  // ESP32 should be sleeping all the time, event handling is done in setup() 😴 
}