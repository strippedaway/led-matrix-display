#include "mqtt.h"
#include "WiFi.h"
#include "config.h"

#include <AsyncMqttClient.h>

TimerHandle_t mqttReconnectTimer;

AsyncMqttClient mqttClient;

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

  char *topic_suc;
  char *topic_fail;
  asprintf(&topic_suc, "%s/%s", MQTT_TOPIC, "success");
  asprintf(&topic_fail, "%s/%s", MQTT_TOPIC, "failed");

  if(strcmp(topic, topic_suc) == 0) {
  } else if(strcmp(topic, topic_fail) == 0) {
  }
}

void onMqttConnect(bool sessionPresent) {
    //DEBUG_PRINT("Connected to MQTT.\n");
    char *topic_suc;
    char *topic_fail;
    asprintf(&topic_suc, "%s/%s", MQTT_TOPIC, "success");
    asprintf(&topic_fail, "%s/%s", MQTT_TOPIC, "failed");
    mqttClient.subscribe(topic_suc, 2);
    mqttClient.subscribe(topic_fail, 2);
    mqttClient.publish(topic_will, 1, true, "online");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  //DEBUG_PRINT("Disconnected from MQTT.\n");
  if (WiFi.isConnected()) {
    StartMQTT();
  }
}

void connectToMQTT() {
  //DEBUG_PRINT("Connecting to MQTT...\n");
  mqttClient.connect();
}



void InitMQTT()
{
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
    mqttClient.setKeepAlive(5);
}
