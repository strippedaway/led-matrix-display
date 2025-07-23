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

extern int16_t space_open;
extern int16_t wc_occupied; //rename to wc2 when add second wc sensor
// extern int16_t wc1_occupied; 

//(when second sensor implemented)
//possible placeholders for sensor info gathering (replace as needed)

//extern int16_t ac_up;
//extern int16_t ac_orgy;

//extern int16_t temp_down;
//extern int16_t temp_up;

extern int16_t displayType;
extern int16_t oldDisplayType;
extern int16_t co2_ppm;
extern int16_t people_inside;
extern int16_t power_watts;
extern char textMsg[2048];