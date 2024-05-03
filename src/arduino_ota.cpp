#include "config.h"
#include <ArduinoOTA.h>
#include "arduino_ota.h"
#include "matrix.h"

TimerHandle_t arduino_ota_timer;
TaskHandle_t arduino_ota_task;

void HandleArduinoOTA(TimerHandle_t) {
  //DEBUG_PRINT("OTA handling\n");
  ArduinoOTA.handle();
}



void InitArduinoOTA() {
  xTaskCreatePinnedToCore(ArduinoOTATask, "arduino_ota_task", 10000, nullptr, 5, &arduino_ota_task, 0);
}

void ArduinoOTATask(void * pvParameters) {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.setPort(OTA_PORT);

  ArduinoOTA.onStart([]() { xTimerStop(arduino_ota_timer, 0); DisableMatrixTimer(); });
  ArduinoOTA.onError([](ota_error_t error) { xTimerStart(arduino_ota_timer, 0); EnableMatrixTimer();  });

  arduino_ota_timer =
      xTimerCreate("arduino_ota_timer", pdMS_TO_TICKS(1000), pdTRUE,
                   nullptr, HandleArduinoOTA);

  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void StartArduinoOTA() {
  ArduinoOTA.begin();
  xTimerStart(arduino_ota_timer, 0);
}

void StopArduinoOTA() {
  ArduinoOTA.end();
}