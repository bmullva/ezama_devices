#include <CAN.h>  // sandeepmistry's arduino-CAN

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4

unsigned long previousMillis = 0;  // Store the last time the message was sent
const long interval = 5000;        // Interval to send message (milliseconds)

void setup() {
  Serial.begin (115200);
  while (!Serial);
  delay (1000);

  Serial.println ("CAN Sender");

  // Set the pins
  CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

  // Start the CAN bus at 50 kbps
  if (!CAN.begin(50000)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if 5 seconds have passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Start packet with ID 0x222
    CAN.beginPacket(0x222);
    // Add data to the packet
    CAN.write(0xDE);
    CAN.write(0xAD);
    CAN.write(0xBE);
    CAN.write(0xEF);
    // End and send the packet
    CAN.endPacket();

    Serial.println("Sent CAN message: 0xDEADBEEF with ID 0x222");
  }

  // Add your receiving code or other tasks here if needed
}
