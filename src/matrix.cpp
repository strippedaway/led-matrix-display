#define PxMATRIX_COLOR_DEPTH   1
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1

#include <PxMatrix.h>
#include "config.h"
#include "matrix.h"

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t matrix_task;

const uint8_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;

uint8_t frame = 0;

PxMATRIX display(WIDTH, HEIGHT, { P_LAT, P_LAT2 }, P_OE, { P_A, P_B });

void IRAM_ATTR display_updater() {
    portENTER_CRITICAL_ISR(&timerMux);
    display.display(32);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void InitMatrix() {

    // init display with 1/4 scan rate
    display.begin(4);

    // clear both buffers
    display.clearDisplay();
    display.showBuffer();
    display.clearDisplay();
    display.showBuffer();

    // init timers
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 1000, true); // 1 ms
    timerAlarmEnable(timer);
}

void StartMatrix() {
  xTaskCreatePinnedToCore(MatrixTask, "matrix_task", 10000, nullptr, 10,
                          &matrix_task, 1);
}

void ShowBuffer() {
    // Block display till buffer is updated
    portENTER_CRITICAL_ISR(&timerMux);
    display.showBuffer();
    portEXIT_CRITICAL_ISR(&timerMux);
}

void DrawFrame() {
    for(uint8_t y = 0; y < 16; ++y) {
        uint8_t pos = (y + 32 - frame) % 32;
        uint16_t c = (pos < 16) ? 15 - pos : (pos - 16);
        display.drawLine(0, y, 32, y, (c << 4));
    }
    display.setTextColor(0x00);
    display.setCursor(1, 1);
    display.println("Hello");
    display.setCursor(2, 8);
    display.println(frame);
    ShowBuffer();

    if(frame == 128) {
        frame = 0;
    }
    frame++;
}

void MatrixTask(void*) {
    DrawFrame();
    vTaskDelay(pdMS_TO_TICKS(100));
}