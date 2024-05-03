void InitMatrix();

void StartMatrix();

void DisplayTask(void *pvParameters);
void MatrixTask(void *pvParameters);

void EnableMatrixTimer();
void DisableMatrixTimer();

extern uint8_t space_open;
extern uint8_t displayType;
extern int16_t co2_ppm;
extern int16_t people_inside;
extern int16_t power_watts;