#include <Wire.h>
#include <Adafruit_MCP23X17.h>

/*
const int GPA0 = 0;   // GPIOA pin 0
const int GPA1 = 1;   // GPIOA pin 1
const int GPA2 = 2;   // GPIOA pin 2
const int GPA3 = 3;   // GPIOA pin 3
const int GPA4 = 4;   // GPIOA pin 4
const int GPA5 = 5;   // GPIOA pin 5
const int GPA6 = 6;   // GPIOA pin 6
const int GPA7 = 7;   // GPIOA pin 7

const int GPB0 = 8;   // GPIOB pin 0
const int GPB1 = 9;   // GPIOB pin 1
const int GPB2 = 10;  // GPIOB pin 2
const int GPB3 = 11;  // GPIOB pin 3
const int GPB4 = 12;  // GPIOB pin 4
const int GPB5 = 13;  // GPIOB pin 5
const int GPB6 = 14;  // GPIOB pin 6
const int GPB7 = 15;  // GPIOB pin 7

A2 A1 A0 
000 = 0x20
001 = 0x21
010 = 0x22
011 = 0x23
100 = 0x24
101 = 0x25
110 = 0x26
111 = 0x27
VSS = GND
VDD = 5V
*/

Adafruit_MCP23X17 mcp;

// Define pins for button input and LED output
const int GPB0 = 8;     // GPA0, bit 1 for LED or any other output device

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect. Needed for native USB

  // Initialize the MCP23017
  delay(500);
  if (!mcp.begin_I2C(0x24)) {
    Serial.println("Error initializing MCP23017");
    while (1); // Loop forever if there's an error
  }
  
  // Set the LED pin as output
  mcp.pinMode(GPB0, OUTPUT);
}

void loop() {
  mcp.digitalWrite(GPB0, HIGH);
  Serial.println("High");
  delay(4000);
  mcp.digitalWrite(GPB0, LOW);
  Serial.println("Low");
  delay(4000);
}
