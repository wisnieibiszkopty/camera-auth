#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "esp_camera.h"
#include "base64.h"
#include "time.h"

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

// todo Refactor as class

void handleError(String message){
    Serial.println(message);
    // set led color to failure
    // play error sound
}

void setTimezone(String timezone){
  Serial.printf("Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  
  tzset();
}

void initTime(String timezone){
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");   
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("Got the time from NTP");
  setTimezone(timezone);
}

String tmToString(const struct tm &t) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             t.tm_year + 1900,
             t.tm_mon + 1,
             t.tm_mday,
             t.tm_hour,
             t.tm_min,
             t.tm_sec);
    return String(buffer);
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
    
    config.frame_size = FRAMESIZE_SXGA; 
    config.jpeg_quality = 6;
    config.fb_count = 1;

    return config;
}

bool verifyFace(String endpoint, camera_fb_t *fb, DeviceData data){
    HTTPClient http;
    http.begin(endpoint);

    String boundary = "----1234567890ABCDEF";
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"photo.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyMiddle = "\r\n--" + boundary + "\r\n";
    bodyMiddle += "Content-Disposition: form-data; name=\"DeviceId\"\r\n\r\n";
    bodyMiddle += data.deviceId + "\r\n";

    bodyMiddle += "--" + boundary + "\r\n";
    bodyMiddle += "Content-Disposition: form-data; name=\"timestamp\"\r\n\r\n";
    bodyMiddle += data.timestamp + "\r\n";

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

        // TODO if response is success grant access

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

bool requestAccess(String endpoint){
    Serial.println("Authorizing...");

    // getting current time
    // for Poland
    initTime("CET-1CEST,M3.5.0/2,M10.5.0/3");
    tm timeinfo;
    getLocalTime(&timeinfo);
    String time = tmToString(timeinfo);
    Serial.println(time);

    // set led color to starting

    // cannot send photo to backend without wifi
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
    //Serial.println(encoded);

    DeviceData data = DeviceData();
    data.deviceId = ESP.getEfuseMac();
    data.timestamp = mktime(&timeinfo);

    boolean authorized = verifyFace(endpoint, fb, data);

    esp_camera_fb_return(fb);
    WiFi.disconnect();

    if(authorized){
        // if face is authorized
        // play succes sound
        // set led color to allowed
        // return true
        return true;
    } else {
        // if face is unauthorized
        // play failure sound
        // set led color to denied
        // return false
        return false;
    }
}