#define PxMATRIX_COLOR_DEPTH   5
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1
#define PxMATRIX_GAMMA_PRESET  3

#include <PxMatrix.h>
#include "config.h"
#include "matrix.h"
#include "debug.h"
#include <Arduino.h>

hw_timer_t * timer = NULL;


SemaphoreHandle_t dispSem = NULL;
static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t matrix_task;
TaskHandle_t display_task;

const uint8_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;

bool timerEnabled = false;

uint8_t displayType;

PxMATRIX display(WIDTH, HEIGHT, { P_LAT, P_LAT2 }, P_OE, { P_A, P_B });

void IRAM_ATTR display_timer() {
    xSemaphoreGiveFromISR(dispSem, NULL);
}

void DisableMatrixTimer() {
    timerAlarmDisable(timer);
    timerEnabled = false;
}

void EnableMatrixTimer() {
    timerAlarmEnable(timer);
    timerEnabled = true;
}

void InitMatrix() {

    DEBUG_PRINT("Init Matrix...");

    // init display with 1/4 scan rate
    display.begin(4);

    // clear both buffers
    display.clearDisplay();
    display.showBuffer();
    display.clearDisplay();
    display.showBuffer();
    display.setBrightness(255);

    displayType = 0;

    // init timers 
    timer = timerBegin(3, 80, true);
    timerAttachInterrupt(timer, &display_timer, true);
    timerAlarmWrite(timer, 1000, true); // 1 ms
    EnableMatrixTimer();

    dispSem = xSemaphoreCreateBinary();

    DEBUG_PRINT(" OK\n");
    xSemaphoreGive(dispSem);
}

void StartMatrix() {
  DEBUG_PRINT("Start Matrix\n");
  xTaskCreatePinnedToCore(DisplayTask, "display_task", 10000, nullptr, 5, &display_task, 0);
  xTaskCreatePinnedToCore(MatrixTask, "matrix_task", 10000, nullptr, 5, &matrix_task, 1);
}

void ShowBuffer() {
    display.showBuffer();
}

void drawCentreString(const char *buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - w / 2, y);
    display.print(buf);
}

void DrawFrame() {
    if(displayType == 0) {
        display.clearDisplay();
        display.setTextColor(0xFF);
        display.setCursor(0, 1);
        display.println("Init...");
        ShowBuffer();
    } else if ( displayType == 1) {
        display.clearDisplay();
        display.setTextColor(0xFF);
        display.setCursor(0, 1);
        display.println("Connecting to Wi-Fi...");
        ShowBuffer();
    } else if ( displayType == 2) {
        display.clearDisplay();
        display.setTextColor(0xFF);
        display.setCursor(0, 1);
        display.println("Connecting to MQTT...");
        ShowBuffer();
    } else if ( displayType == 3) {
        display.clearDisplay();
        display.setTextColor(0xFF);
        display.setCursor(0, 1);
        display.println("OTA...");
        ShowBuffer();
    } else if ( displayType == 4) {
    } else if ( displayType == 5) {
    } else if ( displayType == 6) {
        display.clearDisplay();
        display.setTextColor(0x41);
        display.setCursor(40, 1);
        drawCentreString("Hacker Embassy", 96, 2);
        display.setTextColor(0x80);
        display.setCursor(5, 16);
        display.println(millis());
        ShowBuffer();
    }

}

void MatrixTask(void *pvParameters) {
    (void) pvParameters;

    DEBUG_PRINT("MatrixTask started on core %d\n", xPortGetCoreID());
    for (;;) {
        if(xSemaphoreTake(dispSem, portMAX_DELAY) == pdTRUE) {
            portENTER_CRITICAL_ISR(&timerMux);
            display.display(64);
            portEXIT_CRITICAL_ISR(&timerMux);
        }
    }
}

void DisplayTask(void *pvParameters) {
    (void) pvParameters;

    DEBUG_PRINT("DisplayTask started on core %d\n", xPortGetCoreID());

    for (;;) {
        DrawFrame();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}