#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Arduino.h>

#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN   13  // or SCK
#define CS_PIN    10  // or SS
#define DATA_PIN  11  // or MOSI

void displaySetup();
void printText(uint8_t modStart, uint8_t modEnd, const char *pMsg, bool resetScreen = false);