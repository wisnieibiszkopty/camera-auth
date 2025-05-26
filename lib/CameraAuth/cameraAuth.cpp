#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include "esp_camera.h"
#include "base64.h"

#include "structs.h"

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void handleError(String message){
    Serial.println(message);
    // set led color to failure
    // play error sound
}

camera_config_t getCameraConfig(){
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    config.frame_size = FRAMESIZE_SVGA; 
    config.jpeg_quality = 12;
    config.fb_count = 1;

    return config;
}

static std::optional<RabbitConfig> getConfig(String apiUrl, String connectionString){
    HTTPClient http;

    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"ConnectionString\": \"" + String(connectionString) + "\"}";

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

bool verifyFace(camera_fb_t *fb, const String& jsonMetadata){
    String serverUrl = "";
    HTTPClient http;
    http.begin(serverUrl);

    String boundary = "----1234567890ABCDEF";
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    // Budujemy body
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"photo.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyMiddle = "\r\n--" + boundary + "\r\n";
    bodyMiddle += "Content-Disposition: form-data; name=\"metadata\"\r\n\r\n";
    bodyMiddle += jsonMetadata + "\r\n";

    String bodyEnd = "--" + boundary + "--\r\n";

    size_t bodyLen = bodyStart.length() + fb->len + bodyMiddle.length() + bodyEnd.length();

    uint8_t *body = new uint8_t[bodyLen];
    memcpy(body, bodyStart.c_str(), bodyStart.length());
    memcpy(body + bodyStart.length(), fb->buf, fb->len);
    memcpy(body + bodyStart.length() + fb->len, bodyMiddle.c_str(), bodyMiddle.length());
    memcpy(body + bodyStart.length() + fb->len + bodyMiddle.length(), bodyEnd.c_str(), bodyEnd.length());

    http.addHeader("Content-Length", String(bodyLen));

    int httpCode = http.POST(body, bodyLen);
    delete[] body;

    if (httpCode > 0) {
        Serial.printf("HTTP response code: %d\n", httpCode);
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        http.end();
        return true;
    } else {
        Serial.printf("HTTP POST failed: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }
}

bool requestAccess(String apiUrl, String connectionString){
    Serial.println("Authorizing...");

    // set led color to starting

    if(WiFi.status() != WL_CONNECTED){
        handleError("Wifi isn't connected");
        return false;
    }

    // taking photo with camera
    camera_config_t cameraConfig = getCameraConfig();

    if (esp_camera_init(&cameraConfig)) {
        Serial.println("Camera init failed");
        return false;
    } 

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to get frame");
        return false;
    }

    //String encoded = base64::encode(fb->buf, fb->len);
    //Serial.println("Base64 encoded image:");
    // Serial.println(encoded);

    String jsonMetadata = "{";
        jsonMetadata += "\"DeviceId\":\"esp32-cam-001\",";
        jsonMetadata += "\"timestamp\":" + String(123456789);
        jsonMetadata += "}";

    verifyFace(fb, jsonMetadata);
    esp_camera_fb_return(fb);

    // get config
    // auto rabbitConfig = getConfig(apiUrl, connectionString);

    // if(!rabbitConfig.has_value()){
    //     handleError("Cannot get configuration");
    //     return false;
    // }

    // auto config = rabbitConfig.value();

    // WiFiClient espClient;
    // PubSubClient mqttClient(espClient);
    
    // // connect to rabbit -> maybe add led color for processing error
    // // I need uint

    // mqttClient.setServer(config.url.c_str(), (uint16_t) config.port.toInt());
    // mqttClient.setCallback(mqttCallback);

    // bool connected = mqttClient.connect("ClientID", config.login.c_str(), config.password.c_str());

    // if(!connected){
    //     Serial.println("Not connected to mqtt");
    //     return false;
    // }

    // Serial.println("connected to mqtt");

    // DynamicJsonDocument doc(64 * 1024);

    // doc["deviceId"] = WiFi.macAddress();

    // // replace with ntp server 
    // doc["timestamp"] = esp_timer_get_time();

    // // Forsen
    // // image is to big to be send via mqtt 💀
    // String image = "/9j/4AAQSkZJRgABAQEAAAAAAAD/";
    // doc["base64"] = image;

    // // todo make json from 
    // String json;
    // serializeJson(doc, json);

    // Serial.println(json);

    // mqttClient.publish("verify/face", json.c_str());

    // mqttClient.disconnect();

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