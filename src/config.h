#ifndef MCU_CONFIG_H_
#define MCU_CONFIG_H_

#include "secrets.h"

#define DEBUG_ENABLED
#define DEBUG_SERIAL Serial

#define FS_USE_LITTLEFS

// LED Matrix
#define P_A    22
#define P_B    21
#define P_OE   25
#define P_LAT  32
#define P_LAT2 33
// #define P_DATA 26
// #define P_CLK  18

// WiFi
#define WIFI_SSID "MOWMeOW"
#define WIFI_PASSWORD SECRET_WIFI_PASSWORD

// WiFi
#define OTA_HOSTNAME "led-matrix"
#define OTA_PASSWORD SECRET_OTA_PASSWORD
#define OTA_PORT 3232

// MQTT
#define MQTT_HOST "minimi.lan"
#define MQTT_USERNAME "acs-reader"
#define MQTT_TOPIC "acs"

#endif