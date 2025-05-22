#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// #include "secrets.h"
#define CONFIG_API_URL ""
#define CONFIG_CONNECTION_STRING ""

 #include "structs.h"

void handleError(String message){
    Serial.println(message);
    // set led color to failure
    // play error sound
}

static std::optional<RabbitConfig> getConfig(){
    HTTPClient http;

    String url = CONFIG_API_URL;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"ConnectionString\": \"" + String(CONFIG_CONNECTION_STRING) + "\"}";

    int httpResponseCode = http.POST(json);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);

      RabbitConfig config;
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);

      if(!error){
          config.login = doc["Rabbit:Login"] | "";
          config.password = doc["Rabbit:Password"] | "";
          config.port = doc["Rabbit:Port"] | "";
          config.url = doc["Rabbit:Url"] | "";

          Serial.println(config.login);
          Serial.println(config.password);
          Serial.println(config.url);
          Serial.println(config.port);

          return config;
      }

      return std::nullopt;
    } else {
      Serial.print("Http error: ");
      Serial.println(httpResponseCode);
      return std::nullopt;
    }

    return std::nullopt;
}

void mqttCallback(char* topic, byte* payload, unsigned int length){
    
}

bool requestAccess(){
    Serial.println("Authorizing...");

    // set led color to starting

    if(WiFi.status() != WL_CONNECTED){
        handleError("Wifi isn't connected");
        return false;
    }

    // // get config
    auto rabbitConfig = getConfig();

    if(!rabbitConfig.has_value()){
        handleError("Cannot get configuration");
        return false;
    }

    auto config = rabbitConfig.value();

    WiFiClient espClient;
    PubSubClient mqttClient(espClient);
    
    // connect to rabbit -> maybe add led color for processing error
    // I need uint

    mqttClient.setServer(config.url.c_str(), (uint16_t) config.port.toInt());
    mqttClient.setCallback(mqttCallback);

    bool connected = mqttClient.connect("ClientID", config.login.c_str(), config.password.c_str());

    if(!connected){
        Serial.println("Not connected to mqtt");
        return false;
    }

    Serial.println("connected to mqtt");

    DynamicJsonDocument doc(64 * 1024);

    doc["deviceId"] = WiFi.macAddress();

    // replace with ntp server 
    doc["timestamp"] = esp_timer_get_time();

    // Forsen
    // image is to big to be send via mqtt 💀
    String image = "/9j/4AAQSkZJRgABAQEAAAAAAAD/";
    doc["base64"] = image;

    // todo make json from 
    String json;
    serializeJson(doc, json);

    Serial.println(json);

    mqttClient.publish("verify/face", json.c_str());

    mqttClient.disconnect();
    WiFi.disconnect();

    // set led color to processing
    // take picture
    // convert to base64
    // add additional info to json
    // send to server
    // wait for response

    // if face is authorized
        // play succes sound
        // set led color to allowed
        // return true

    // if face is unauthorized
        // play failure sound
        // set led color to denied
        // return false

    return true;
}