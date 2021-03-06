#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP23017.h>
#include "65C02S_Mnemonics.h"

#define SERIAL_BAUD 115200
#define CLOCK 2
#define MCP1_ADDR 0
#define MCP2_ADDR 1

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MCP23017 mcp1, mcp2;

volatile byte dataValue;
volatile unsigned int addressValue;
volatile boolean rwb;
volatile boolean sync;
volatile boolean clockTicked;
String output;

void setup() {
  output.reserve(256);
  Serial.begin(SERIAL_BAUD);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextColor(SSD1306_WHITE);

  mcp1.begin(MCP1_ADDR, &Wire);
  mcp2.begin(MCP2_ADDR, &Wire);

  // Enable pullups on unused pins
  for (int i = 10; i <= 15; i++) {
    mcp2.pullUp(i, HIGH);
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK), setClockTicked, RISING);
}

void setClockTicked() {
  clockTicked = true;
}

void updateValues() {
  addressValue = mcp2.readGPIOAB();
  dataValue = mcp1.readGPIO(0);
  rwb = mcp1.digitalRead(8);
  sync = mcp1.digitalRead(9);
}

void loop() {
  if (clockTicked) {
    updateValues();
    clockTicked = false;
    
    output = String(addressValue, HEX);
    output += " ";
    output += String(rwb ? 'r' : 'W');
    output += " ";
    output += sync ? String(mnemonic[dataValue]) : String(dataValue, HEX);
    
    Serial.println(output);
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("AB   RW DB"));
    display.println(output);
    display.display();
  }
}
