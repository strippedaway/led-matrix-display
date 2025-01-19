#define PxMATRIX_COLOR_DEPTH   8
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1
#define PxMATRIX_GAMMA_PRESET  3

#include <PxMatrix.h>
#include "config.h"
#include "matrix.h"
#include "net_frames.h"
#include "debug.h"
#include <Arduino.h>
#include "time.h"
#include "fonts/CourierCyr10.h"
#include "fonts/FreeSansBold10.h"
#include "arduino_ota.h"
#include "net_frames.h"

hw_timer_t * timer = NULL;

SemaphoreHandle_t dispSem = NULL;
SemaphoreHandle_t vSyncSem = NULL;

static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t matrix_task;
TaskHandle_t display_task;

char textMsg[2048] = {0};

int16_t xScrollPos = 0;

bool timerEnabled = false;
bool stopScroll = true;

int16_t displayType;
int16_t oldDisplayType;

int16_t wc_occupied;
int16_t space_open;
int16_t co2_ppm;
int16_t people_inside;
int16_t power_watts;

uint32_t latestBlink;
bool blinkState = false;
bool displayTcp = false;

PxMATRIX display(WIDTH, HEIGHT, { P_LAT, P_LAT2 }, P_OE, { P_A, P_B });

void IRAM_ATTR display_timer() {
    xSemaphoreGiveFromISR(dispSem, NULL);
}

void ResetTextScroll() {
    xScrollPos = WIDTH+2;
    stopScroll = true;
}

void DisableMatrixTimer() {
    // will stop after screen is refreshed
    timerEnabled = false;
}

void drawBitmapBuffer() {
    displayTcp = true;
    display.clearDisplay();
    display.drawGrayscaleBitmap(0, 0, imageBuffer, WIDTH, HEIGHT);
    ShowBuffer();
}

void EnableMatrixTimer() {
    timerRestart(timer);
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
    //display.setFastUpdate(true);

    latestBlink = millis();

    displayType = 0;
    space_open = -1;
    co2_ppm = -1;
    people_inside = -1;
    power_watts = -1;

    // init timers 
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &display_timer);
    timerAlarm(timer, 1000, true, 0); // 1 ms
    EnableMatrixTimer();

    dispSem = xSemaphoreCreateBinary();
    //vSyncSem = xSemaphoreCreateBinary();

    DEBUG_PRINT(" OK\n");
    xSemaphoreGive(dispSem);
    //xSemaphoreGive(vSyncSem);
}

void StartMatrix() {
  DEBUG_PRINT("Start Matrix\n");
  xTaskCreatePinnedToCore(DisplayTask, "display_task", 10000, nullptr, 5, &display_task, 0);
  xTaskCreatePinnedToCore(MatrixTask, "matrix_task", 10000, nullptr, 1, &matrix_task, 1);
}

void ShowBuffer() {
    // xSemaphoreTake(vSyncSem, pdMS_TO_TICKS(10));
    display.showBuffer();
    // xSemaphoreGive(vSyncSem);
}

void drawCentreString(const char *buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - w / 2, y);
    display.print(buf);
}

void blinkColor(uint32_t delayBlink) {
    if(millis() - latestBlink > delayBlink) {
        blinkState = ! blinkState;
        latestBlink = millis();
    }
    display.setTextColor(blinkState ? 0x41 : 0x80);
}

void ScrollText() {
    display.setTextColor(0x41);
    display.setTextWrap(false);
    display.setFont(&FreeSansBold10pt8b);
    
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(textMsg, 0, 20, &x1, &y1, &w, &h); //calc width of string

    const int16_t startX = WIDTH+1;
    const int16_t endX = (-15 - w);

    // display.setCursor(0, 32);
    // display.print(startX);
    // display.print(" ");
    // display.print(endX);
    // ShowBuffer();

    stopScroll = false;
    int xScrollShift = endX - startX + 15;
    float a;
    
    if(w > 1) {
        for(xScrollPos = startX; xScrollPos > endX; xScrollPos--) {
            if(stopScroll) break;

            a = 1-float(xScrollPos-startX)/xScrollShift;

            // display.setFont(&FreeSansBold10pt8b);
            display.setCursor(xScrollPos, 22);
            display.print(textMsg);
            display.fillRect(0, 30, a * WIDTH, 2, 0x81);

            // display.setFont();
            // display.setCursor(0, 25);
            // display.print(xScrollPos);
            ShowBuffer();
            vTaskDelay(pdMS_TO_TICKS(20));
            display.clearDisplay();
        }
    }
    display.setFont();
}

void DrawFrame() {
    if(!displayTcp && timerEnabled) {
        
        bool timeKnown;
        struct tm timeinfo;
        
        if(!getLocalTime(&timeinfo)) timeKnown = false;
        else timeKnown = true;

        display.clearDisplay();
        display.setTextColor(0xFF);
        display.setTextSize(1);

        if(displayType == 0) {
            display.setCursor(0, 1);
            display.println("Init...");
            ShowBuffer();
        } else if ( displayType == 1) {
            for(int i = 0; i <= 3; i++) {
                display.setCursor(0, 1);
                display.print("Connecting to Wi-Fi");
                for(int a = 0; a < i; a++) display.print(".");
                ShowBuffer();
                display.clearDisplay();
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        } else if ( displayType == 2) {
            for(int i = 0; i <= 3; i++) {
                display.setCursor(0, 1);
                display.print("Connecting to MQTT");
                for(int a = 0; a < i; a++) display.print(".");
                ShowBuffer();
                display.clearDisplay();
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        } else if ( displayType == 3) {
            for(int i = 0; i <= 3; i++) {
                display.setCursor(0, 1);
                display.print("OTA");
                for(int a = 0; a < i; a++) display.print(".");
                if(otaProgress < 200) {
                    display.drawRect(0, 16, WIDTH, 5, 0x41);
                    display.fillRect(1, 17, int((float(WIDTH-1)/100)*otaProgress), 3, 0x81);
                }
                ShowBuffer();
                display.clearDisplay();
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        } else if ( displayType == 4) {
            ShowBuffer();
        } else if ( displayType == 5) {
            display.setTextColor(0x41);
            display.setCursor(40, 1);
            drawCentreString("Hacker Embassy", 96, 2);
            display.setTextColor(0x80);
            display.setCursor(5, 16);
            display.println(millis());
            ShowBuffer();
        } else if ( displayType == 6) {

            display.setTextColor(0x41);

            if(space_open != -1) {
                if(space_open == 1) {
                    display.setTextColor(0x80);
                    display.setCursor(165, 2);
                    display.print("Open");
                    display.setTextColor(0x41);
                } else if(space_open == 0) {
                    display.setCursor(155, 2);
                    display.print("Closed");
                }
            }


            if(wc_occupied == 1) {
                blinkColor(1000);
                display.setCursor(70, 2);
                display.print("WC occupied");
                display.setTextColor(0x41);
            } else if(wc_occupied == 0) {
                display.setCursor(62, 2);
                display.print("Hacker Embassy");
            } else if(wc_occupied == -1) {
                blinkColor(500);
                display.setCursor(70, 2);
                display.print("WC offline");
                display.setTextColor(0x41);
            }

            if(timeKnown) {
                display.setCursor(3, 2);
                display.print(&timeinfo, "%H:%M:%S");
            }
            
            display.setCursor(2, 16);
            if(co2_ppm > 1500) blinkColor(500); 
            if(co2_ppm != -1) display.printf("CO2: %d", co2_ppm);
            display.setTextColor(0x41);

            display.setCursor(76, 16);
            if(people_inside != -1) display.printf("Inside: %d", people_inside);

            display.setCursor(150, 16);
            if(power_watts != -1) display.printf("%d W", power_watts);
            ShowBuffer();


        } else if ( displayType == 7) {
            display.fillRect(0, 0, 192, 32, 0x81);
            ShowBuffer();
        } else if ( displayType == 8) {
            display.setTextColor(0x41);
            if(timeKnown) {
                if(timeinfo.tm_yday >= 365 && timeinfo.tm_hour == 23 && timeinfo.tm_min >= 59 && (timeinfo.tm_sec % 2) == 0) {
                    display.fillRect(0, 0, 192, 32, 0x81);
                    display.setTextColor(0x0);
                }
                display.setTextSize(3);
                display.setCursor(25, 5);
                display.print(&timeinfo, "%H:%M:%S");
            } else {
                display.setTextSize(2);
                display.setCursor(20, 14);
                display.print("time not set");
            }
            ShowBuffer();
        } else if ( displayType == 9) { // scrolltest
            ScrollText();
        } else if ( displayType == 10) { // scrolltest
            ScrollText();
            displayType = oldDisplayType;
        }
    } else if (!timerEnabled && timerRead(timer) != 0) {
        display.clearDisplay();
        ShowBuffer();
        display.clearDisplay();
        ShowBuffer();
        vTaskDelay(pdMS_TO_TICKS(300));
        timerStop(timer);
        timerWrite(timer, 0);
    }
}

void MatrixTask(void *pvParameters) {
    (void) pvParameters;

    DEBUG_PRINT("MatrixTask started on core %d\n", xPortGetCoreID());
    for (;;) {
        if(xSemaphoreTake(dispSem, pdMS_TO_TICKS(50)) == pdTRUE) {
            portENTER_CRITICAL_ISR(&timerMux);
            // while(xSemaphoreTake(vSyncSem, 0) != pdTRUE) {
                // vTaskDelay(2);
            // }
            display.display(8);
            // xSemaphoreGive(vSyncSem);
            //if(!timerEnabled) timerAlarmDisable(timer);
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