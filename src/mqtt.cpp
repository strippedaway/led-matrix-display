#include "mqtt.h"
#include "WiFi.h"
#include "config.h"
#include "debug.h"
#include "matrix.h"
#include <cstring> // for strncpy
#include <string>  // for std::string
#include <cstdlib> // for std::atoi


#include <AsyncMqttClient.h>

TimerHandle_t mqttReconnectTimer;

AsyncMqttClient mqttClient;

#define PEOPLE_COUNT_TOPIC "hass-states/sensor/people_inside/state"
#define SPACE_OPEN_TOPIC "hass-states/binary_sensor/hackerspace_open/state"
#define SPACE_POWER_TOPIC "hass-states/sensor/hackem_powermeter_bl0942_power/state"
#define DOWNSTAIRS_CO2_TOPIC "hass-states/sensor/downstairs_co2_value/state"
#define WC_OCCUPIED_TOPIC "hass-states/binary_sensor/toilet_presence/state"
#define MATRIX_ENABLED_TOPIC "hass-states/input_boolean/matrix_enabled/state"
#define MATRIX_TEXT_TOPIC "led-matrix/display-text"
#define MATRIX_DEBUGSCREEN_TOPIC "led-matrix/debug-screen"
#define HOME_ASSISTANT_STATUS_TOPIC "homeassistant/status"

char *topic_will;

void PublishToMQTT(const char* type, const char* data) {
    char *topic;
    asprintf(&topic, "%s/%s", MQTT_TOPIC, type);
    mqttClient.publish(topic, 2, false, data);
}

void StopMQTT() {
    xTimerStop(mqttReconnectTimer, 0);
}

void StartMQTT() {
    xTimerStart(mqttReconnectTimer, 0);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  bool availableData = true;
  if(strncmp(payload, "unavailable", len) == 0) {
    availableData = false;
  }

  std::string strPayload(payload, len);
  int16_t numPayload = -1;
  if(availableData) numPayload = (std::atof(strPayload.c_str()));


  if(strcmp(topic, PEOPLE_COUNT_TOPIC) == 0) {
    people_inside = numPayload;
  } else if(strcmp(topic, SPACE_OPEN_TOPIC) == 0) {
    if(availableData)
      space_open = (strncmp(payload, "on", len) == 0) ? 1 : 0;
    else
      space_open = -1;
  } else if(strcmp(topic, WC_OCCUPIED_TOPIC) == 0) {
    if(availableData)
      wc_occupied = (strncmp(payload, "on", len) == 0) ? 1 : 0;
    else
      wc_occupied = -1;
  } else if(strcmp(topic, HOME_ASSISTANT_STATUS_TOPIC) == 0) {
    if(strncmp(payload, "online", len) == 0) {
      if(displayType == 33)
        displayType = 6;
    } else if(strncmp(payload, "offline", len) == 0) {
      if(displayType != 9 && displayType != 10 && displayType != 33) {
        oldDisplayType = displayType;
        displayType = 33;
      }
    }
  } else if(strcmp(topic, SPACE_POWER_TOPIC) == 0) {
    power_watts = numPayload;
  } else if(strcmp(topic, MATRIX_DEBUGSCREEN_TOPIC) == 0) {
    if(numPayload == 20) {
      displayTcp = true;
    } else {
      displayTcp = false;
      displayType = numPayload;
    }
    if(numPayload == -69) ESP.restart();
  } else if(strcmp(topic, DOWNSTAIRS_CO2_TOPIC) == 0) {
    co2_ppm = numPayload;
  } else if(strcmp(topic, MATRIX_ENABLED_TOPIC) == 0) {
    if(strncmp(payload, "off", len) == 0) {
      DisableMatrixTimer();
    } else {
      EnableMatrixTimer();
    }
  } else if(strcmp(topic, MATRIX_TEXT_TOPIC) == 0) {
    //textMsg[0] = '\0';
    if(len < 1024) {
      memset(textMsg, 0, strlen(textMsg));
      strncpy(textMsg, payload, len);
      ResetTextScroll();
      if(displayType != 9 && displayType != 10) {
        oldDisplayType = displayType;
        displayType = 10;
      }
    }
  }
}

void onMqttConnect(bool sessionPresent) {
    DEBUG_PRINT("Connected to MQTT.\n");
    displayType = 6;
    mqttClient.subscribe(PEOPLE_COUNT_TOPIC, 1);
    mqttClient.subscribe(SPACE_OPEN_TOPIC, 1);
    mqttClient.subscribe(SPACE_POWER_TOPIC, 1);
    mqttClient.subscribe(DOWNSTAIRS_CO2_TOPIC, 1);
    mqttClient.subscribe(WC_OCCUPIED_TOPIC, 1);
    mqttClient.subscribe(MATRIX_TEXT_TOPIC, 1);
    mqttClient.subscribe(MATRIX_ENABLED_TOPIC, 1);
    mqttClient.subscribe(MATRIX_DEBUGSCREEN_TOPIC, 1);
    mqttClient.subscribe(HOME_ASSISTANT_STATUS_TOPIC, 1);
    mqttClient.publish(topic_will, 1, true, "online");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  DEBUG_PRINT("Disconnected from MQTT.\n");
  if (WiFi.isConnected()) {
    StartMQTT();
  }
}

void connectToMQTT() {
  displayType = 2;
  DEBUG_PRINT("Connecting to MQTT...\n");
  mqttClient.connect();
}


void InitMQTT()
{
  DEBUG_PRINT("Init MQTT\n");
  asprintf(&topic_will, "%s/%s", MQTT_TOPIC, "lwt");

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMQTT));
  StopMQTT();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, 1883);
  mqttClient.setClientId(OTA_HOSTNAME);
  mqttClient.setCredentials(MQTT_USERNAME, SECRET_MQTT_PASSWORD);
  mqttClient.setWill(topic_will, 1, true, "offline");
  mqttClient.setKeepAlive(15);
}
