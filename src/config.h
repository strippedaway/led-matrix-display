#include <Arduino.h>
#ifndef MCU_CONFIG_H_
#define MCU_CONFIG_H_

#include "secrets.h"

#define DEBUG_ENABLED
#define DEBUG_SERIAL Serial

#define FS_USE_LITTLEFS

// LED Matrix params
const uint8_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;

// LED Matrix pinout
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
#define OTA_HOSTNAME "led-matrix-red"
#define OTA_PASSWORD SECRET_OTA_PASSWORD
#define OTA_PORT 3232

// MQTT
#define MQTT_HOST "hass.lan"
#define MQTT_USERNAME "led-matrix-red"
#define MQTT_TOPIC "matrix"

#endif