#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "config.h"
#include "matrix.h"
#include "esp_wifi.h"
#include "net_frames.h"
//#include <vector>

/***************************************************
 *  Async TCP Server Setup
 ***************************************************/
AsyncServer* tcpServer = nullptr;

const uint16_t SERVER_PORT = 1234;  // Change to any preferred port


// For a single frame, we need WIDTH * HEIGHT bytes
uint8_t imageBuffer[WIDTH * HEIGHT];
const size_t MAX_FRAME_SIZE = sizeof(imageBuffer);

// We'll keep track of our "state": are we reading the 4-byte header or the frame itself?
enum class ReadState {
  READ_HEADER,
  READ_FRAME
};

static ReadState currentState = ReadState::READ_HEADER;
static size_t bytesNeeded     = 4;  // when in READ_HEADER, we need 4 bytes for the header
static size_t frameBytesCount = 0;  // how many bytes of the frame we've received so far
static size_t frameSize       = 0;  // total bytes in the current frame

void InitTCP() {
    tcpServer = new AsyncServer(SERVER_PORT);
    tcpServer->onClient(&handleNewClient, nullptr);
    tcpServer->begin();
}

void handleDisconnect(void* arg, AsyncClient* client) {
  displayTcp = false;
}

void handleTimeout(void* arg, AsyncClient* client, uint32_t timeO) {
  displayTcp = false;
} 

void handleError(void* arg, AsyncClient* client, int8_t errorO) {
  displayTcp = false;
}

// Callback when new data arrives from client
void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
  uint8_t* incomingData = (uint8_t*) data;
  size_t offset = 0;

  while (len > 0) {
    if (currentState == ReadState::READ_HEADER) {
      // We need to gather 4 bytes for the header
      static uint8_t headerBuffer[4];
      size_t toCopy = min(bytesNeeded, len);
      memcpy(&headerBuffer[4 - bytesNeeded], &incomingData[offset], toCopy);
      bytesNeeded -= toCopy;
      offset += toCopy;
      len    -= toCopy;

      // If we've read all 4 bytes of the header
      if (bytesNeeded == 0) {
        // Convert the 4 bytes (big-endian) to an integer
        frameSize = ((uint32_t)headerBuffer[0] << 24) |
                    ((uint32_t)headerBuffer[1] << 16) |
                    ((uint32_t)headerBuffer[2] <<  8) |
                    ((uint32_t)headerBuffer[3] <<  0);

        // Safety check
        if (frameSize > MAX_FRAME_SIZE) {
          //Serial.printf("Frame size too large (%u bytes). Resetting.\n", frameSize);
          // Reset state to read header again
          currentState = ReadState::READ_HEADER;
          bytesNeeded  = 4;
          continue; // discard this frame
        }

        // Prepare to read frameSize bytes
        currentState    = ReadState::READ_FRAME;
        bytesNeeded     = frameSize;
        frameBytesCount = 0;
      }
    }
    else { // currentState == ReadState::READ_FRAME
      // We are reading the frame data
      size_t toCopy = min(bytesNeeded, len);
      memcpy(&imageBuffer[frameBytesCount], &incomingData[offset], toCopy);
      frameBytesCount += toCopy;
      bytesNeeded     -= toCopy;
      offset          += toCopy;
      len             -= toCopy;

      // If we've received the entire frame
      if (bytesNeeded == 0) {
        
        displayTcp = true;
        //Serial.printf("Received full frame (%u bytes). Displaying...\n", frameSize);
        drawBitmapBuffer();

        // Reset to read the next header
        currentState = ReadState::READ_HEADER;
        bytesNeeded  = 4;
      }
    }
  }
}

// Callback when new client connects
void handleNewClient(void* arg, AsyncClient* client) {
  Serial.printf("New client connected from: %s\n", client->remoteIP().toString().c_str());
  // Reset read state for new connection
  currentState = ReadState::READ_HEADER;
  bytesNeeded  = 4;
  frameBytesCount = 0;
  frameSize    = 0;

  if(displayTcp) {
    client->abort();
    return;
  }
  // Attach data callback
  client->setAckTimeout(10000);
  client->setRxTimeout(10000);
  client->setKeepAlive(3000, 3);
  client->onData(&handleData, nullptr);
  client->onDisconnect(&handleDisconnect, nullptr);
  client->onTimeout(&handleTimeout, nullptr);
  client->onError(&handleError, nullptr);
}
