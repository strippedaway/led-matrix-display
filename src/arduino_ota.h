#ifndef ARDUINO_OTA_H_
#define ARDUINO_OTA_H_

void InitArduinoOTA();
void ArduinoOTATask(void *pvParameters);
void StartArduinoOTA();
void StopArduinoOTA();

#endif