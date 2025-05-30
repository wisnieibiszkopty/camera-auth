#ifndef FACEAUTHSYSTEM_H
#define FACEAUTHSYSTEM_H

#include <Arduino.h>
#include <HTTPClient.h>
#include "esp_camera.h"
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

class FaceAuthSystem {
public:
    FaceAuthSystem(const String &endpoint, const String &timezone);
    bool requestAccess();
    ~FaceAuthSystem();

private:
    String endpoint;
    camera_config_t cameraConfig;

    uint64_t deviceId;

    void handleError(const String &message);
    void setTimezone(const String &timezone);
    void initTime(const String &timezone);
    String tmToString(const struct tm &t);
    camera_config_t getCameraConfig();
    bool verifyFace(camera_fb_t *fb, const DeviceData &data);
};

#endif