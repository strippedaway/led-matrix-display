#ifndef ARDUINO_OTA_H_
#define ARDUINO_OTA_H_

extern uint8_t otaProgress;

void InitArduinoOTA();
void ArduinoOTATask(void *pvParameters);
void StartArduinoOTA();
void StopArduinoOTA();

#endif