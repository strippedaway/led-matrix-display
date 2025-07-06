#include "config.h"
#include <ArduinoOTA.h>
#include "arduino_ota.h"
#include "matrix.h"
#include "mqtt.h"
#include "mqtt.h"

TaskHandle_t arduino_ota_task;

bool otaEnabled = false;
uint8_t otaProgress = 255;

void InitArduinoOTA() {
  xTaskCreatePinnedToCore(ArduinoOTATask, "arduino_ota_task", 10000, nullptr, 12, &arduino_ota_task, 0);
}

void ArduinoOTATask(void * pvParameters) {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.setPort(OTA_PORT);

  ArduinoOTA.onStart([]() { displayType = 3; StopMQTT(); otaProgress = 255; });
  ArduinoOTA.onError([](ota_error_t error) { displayType = 6; StartMQTT(); otaProgress = 255; });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaProgress = progress / (total / 100);
  });


  for(;;) {
    if(otaEnabled) ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void StartArduinoOTA() {
  ArduinoOTA.begin();
  otaEnabled = true;
}

void StopArduinoOTA() {
  ArduinoOTA.end();
  otaEnabled = false;
}