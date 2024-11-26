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

// Define which pin on the MCP23017 we're using for the button
const int GPB0 = 8;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect. Needed for native USB

  // Initialize the MCP23017
  delay(500);
  if (!mcp.begin_I2C(0x24)) {
    Serial.println("Error initializing MCP23017");
    while (1); // Loop forever if there's an error
  }
  
  // Set the button pin as input with pull-up
  mcp.pinMode(GPB0, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Read the button state. Note that stopped because it's an active low switch, 
  // we'll detect a press when the reading is LOW
  int buttonState = mcp.digitalRead(GPB0);

  if (buttonState == LOW) {
    Serial.println("Button Pressed");
    digitalWrite(LED_BUILTIN, HIGH);
    // You might want to add debounce logic here for real applications
    while (mcp.digitalRead(GPB0) == LOW); // Wait for release
    Serial.println("Button Released");
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  delay(50); // Small delay to debounce and not flood the serial monitor
}
