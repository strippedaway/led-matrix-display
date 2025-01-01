void InitMatrix();

void StartMatrix();

void DisplayTask(void *pvParameters);
void MatrixTask(void *pvParameters);

void EnableMatrixTimer();
void DisableMatrixTimer();

void drawBitmapBuffer();

void ShowBuffer();

void ResetTextScroll();

extern bool displayTcp;

const uint8_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;

extern int16_t space_open;
extern int16_t wc_occupied;
extern int16_t displayType;
extern int16_t oldDisplayType;
extern int16_t co2_ppm;
extern int16_t people_inside;
extern int16_t power_watts;
extern char textMsg[2048];