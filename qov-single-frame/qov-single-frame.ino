#include "SD.h"
#include "SPI.h"

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

// OV5640 pins
#define hsync A0
#define vsync A1
#define xclk A2
#define pclk A3
#define d2 5
#define d3 6
#define d4 9
#define d5 10
#define d6 11
#define d7 12
#define d8 13
#define d9 A4

// SD pins
#define clk SCK
#define cmd MOSI
#define sd_d3 A5

// global data
uint8_t *frame_buffer;

void logMemory() {
  Serial.print("Used PSRAM: "); Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
}

void setup() {
  while(!Serial);
  Serial.println("single frame demo");

  if (!SD.begin(sd_d3)) {
    Serial.println("error, failed mounting SD");
    while(1);
  }
  Serial.println("SD connected");

  // initialize frame buffer
  logMemory();
  frame_buffer = (uint8_t*)ps_malloc(FRAME_WIDTH * FRAME_HEIGHT * 3);
  for (int i = 0; i < sizeof(frame_buffer) / sizeof(uint8_t); i++) {
    frame_buffer[i] = 0;
  }
  logMemory();
  free(frame_buffer);
  logMemory();
}

void loop() {
  // nothing to do in main loop
  delay(1000);
}