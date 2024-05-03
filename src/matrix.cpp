#define PxMATRIX_COLOR_DEPTH   1
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1

#include <PxMatrix.h>
#include "config.h"
#include "matrix.h"
#include "debug.h"
#include <Arduino.h>

hw_timer_t * timer = NULL;


SemaphoreHandle_t dispSem = NULL;
SemaphoreHandle_t updateSem = NULL;
static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t matrix_task;
TaskHandle_t display_task;

const uint8_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;

bool timerEnabled = false;

uint32_t frame = 0;

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

    // init timers 
    timer = timerBegin(3, 80, true);
    timerAttachInterrupt(timer, &display_timer, true);
    timerAlarmWrite(timer, 1000, true); // 1 ms
    EnableMatrixTimer();

    dispSem = xSemaphoreCreateBinary();
    updateSem = xSemaphoreCreateBinary();

    DEBUG_PRINT(" OK\n");
    xSemaphoreGive(dispSem);
    xSemaphoreGive(updateSem);
}

void StartMatrix() {
  DEBUG_PRINT("Start Matrix\n");
  xTaskCreatePinnedToCore(DisplayTask, "display_task", 10000, nullptr, 5, &display_task, 0);
  xTaskCreatePinnedToCore(MatrixTask, "matrix_task", 10000, nullptr, 5, &matrix_task, 1);
}

void ShowBuffer() {
    // Block display till buffer is updated
    if (xSemaphoreTake(updateSem, portMAX_DELAY) == pdTRUE) {
        // DEBUG_PRINT("matrix: showing buffer\n");
        display.showBuffer();
        xSemaphoreGive(updateSem);
    }
    
}

void DrawFrame() {
    display.clearDisplay();
    display.setTextColor(0xFF);
    display.setCursor(32, 1);
    display.println("Ha-hacker Embasissy");
    display.setCursor(2, 16);
    display.setTextColor(0xFF);
    display.println(frame);
    display.
    ShowBuffer();

    if(frame == UINT32_MAX) {
        frame = 0;
    }
    frame++;
}

void MatrixTask(void *pvParameters) {
    (void) pvParameters;

    DEBUG_PRINT("MatrixTask started on core %d\n", xPortGetCoreID());
    for (;;) {
        if(xSemaphoreTake(dispSem, portMAX_DELAY) == pdTRUE) {
            if(xSemaphoreTake(updateSem, portMAX_DELAY) == pdTRUE) {
                portENTER_CRITICAL_ISR(&timerMux);
                display.display(32);
                portEXIT_CRITICAL_ISR(&timerMux);
                xSemaphoreGive(updateSem);
            }
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