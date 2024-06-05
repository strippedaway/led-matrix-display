void InitMatrix();

void StartMatrix();

void DisplayTask(void *pvParameters);
void MatrixTask(void *pvParameters);

void EnableMatrixTimer();
void DisableMatrixTimer();

void ResetTextScroll();

extern int16_t space_open;
extern int16_t wc_occupied;
extern int16_t displayType;
extern int16_t oldDisplayType;
extern int16_t co2_ppm;
extern int16_t people_inside;
extern int16_t power_watts;
extern char textMsg[2048];