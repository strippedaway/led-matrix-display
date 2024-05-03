#include <Arduino.h>
#include "config.h"
#include "arduino_ota.h"
#include "esp_wifi.h"
#include "mqtt.h"
#include "matrix.h"


void setup() {
  InitMatrix();
  InitWiFi();
  InitMQTT();
  InitArduinoOTA();

  StartMatrix();
  StartWiFi();
}

void loop() {}