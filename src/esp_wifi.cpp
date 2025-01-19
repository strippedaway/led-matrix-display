#include "esp_wifi.h"
#include <WiFi.h>
#include "arduino_ota.h"
#include "mqtt.h"
#include "config.h"
#include "debug.h"
#include "matrix.h"
#include "time.h"
#include "net_frames.h"

const char* ntpServerA = "0.am.pool.ntp.org";
const char* ntpServerB = "1.am.pool.ntp.org";
const long  gmtOffset_sec = 14400;

TimerHandle_t wifi_reconnect_timer;


/*
 *  This sketch shows the WiFi event usage
 *
*/

/*
* WiFi Events

0  ARDUINO_EVENT_WIFI_READY               < ESP32 WiFi ready
1  ARDUINO_EVENT_WIFI_SCAN_DONE                < ESP32 finish scanning AP
2  ARDUINO_EVENT_WIFI_STA_START                < ESP32 station start
3  ARDUINO_EVENT_WIFI_STA_STOP                 < ESP32 station stop
4  ARDUINO_EVENT_WIFI_STA_CONNECTED            < ESP32 station connected to AP
5  ARDUINO_EVENT_WIFI_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  ARDUINO_EVENT_WIFI_STA_GOT_IP               < ESP32 station got IP from connected AP
8  ARDUINO_EVENT_WIFI_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  ARDUINO_EVENT_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 ARDUINO_EVENT_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 ARDUINO_EVENT_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 ARDUINO_EVENT_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 ARDUINO_EVENT_WIFI_AP_START                 < ESP32 soft-AP start
14 ARDUINO_EVENT_WIFI_AP_STOP                  < ESP32 soft-AP stop
15 ARDUINO_EVENT_WIFI_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 ARDUINO_EVENT_WIFI_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 ARDUINO_EVENT_WIFI_AP_GOT_IP6               < ESP32 ap interface v6IP addr is preferred
19 ARDUINO_EVENT_WIFI_STA_GOT_IP6              < ESP32 station interface v6IP addr is preferred
20 ARDUINO_EVENT_ETH_START                < ESP32 ethernet start
21 ARDUINO_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 ARDUINO_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 ARDUINO_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 ARDUINO_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
19 ARDUINO_EVENT_ETH_GOT_IP6              < ESP32 ethernet interface v6IP addr is preferred
25 ARDUINO_EVENT_MAX
*/

void ConnectToWiFi(TimerHandle_t) {
  displayType = 1;
  DEBUG_PRINT("Connecting to WiFi...\n");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiEvent(WiFiEvent_t event) {
  DEBUG_PRINT("Got WiFi event: %d\n", event);
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUG_PRINT("WiFi connected\n");
      DEBUG_PRINT("IP address: %s\n", WiFi.localIP().toString().c_str());
      StartArduinoOTA();
      DEBUG_PRINT("OTA server has been started\n");
      xTimerStop(wifi_reconnect_timer, 0);
      StartMQTT();
      configTime(gmtOffset_sec, 0, ntpServerA, ntpServerB);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUG_PRINT("WiFi lost connection\n");
      displayType = 1;
      StopArduinoOTA();
      StopMQTT();
      xTimerStart(wifi_reconnect_timer, 0);
      break;
  }
}

void InitWiFi() {
  DEBUG_PRINT("Init WiFi\n");
  WiFi.mode(WIFI_OFF);
  WiFi.setHostname(OTA_HOSTNAME);
  WiFi.mode(WIFI_STA);
  
  wifi_reconnect_timer = xTimerCreate("wifi_timer", pdMS_TO_TICKS(2000),
                                      pdFALSE, nullptr, ConnectToWiFi);
  WiFi.onEvent(WiFiEvent);
}

void StartWiFi() {
  ConnectToWiFi(nullptr);
}