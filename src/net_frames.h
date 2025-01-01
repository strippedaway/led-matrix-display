#include <AsyncTCP.h>

extern uint8_t imageBuffer[WIDTH * HEIGHT];
extern const size_t MAX_FRAME_SIZE;

void InitTCP();
void handleNewClient(void* arg, AsyncClient* client);