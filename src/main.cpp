#include <Arduino.h>
#include "config.h"
#include "arduino_ota.h"
#include "esp_wifi.h"
#include "mqtt.h"
#include "matrix.h"
#include "debug.h"

#include <FailSafe.h>
const time_t BOOT_FLAG_TIMEOUT = 20000; // Time in ms to reset flag
const int MAX_CONSECUTIVE_BOOT = 5; // Number of rapid boot cycles before enabling fail safe mode
const int LED = 1; // Number of rapid boot cycles before enabling fail safe mode
const int RTC_ADDRESS = 0; // If you use RTC memory adjust offset to not overwrite other data

void setup() {

    FailSafe.checkBoot (MAX_CONSECUTIVE_BOOT, LED, RTC_ADDRESS); // Parameters are optional
    if (FailSafe.isActive ()) { // Skip all user setup if fail safe mode is activated
        return;
    }

  InitDebug();
  InitMatrix();
  InitWiFi();
  InitMQTT();
  InitArduinoOTA();

  StartMatrix();
  StartWiFi();
}

void loop() {
    FailSafe.loop (BOOT_FLAG_TIMEOUT); // Use always this line

    delay(10000);
}