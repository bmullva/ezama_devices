#include <CAN.h>  // sandeepmistry's arduino-CAN

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4

void setup() {
  Serial.begin (115200);
  while (!Serial);
  delay (1000);

  Serial.println ("CAN Receiver");

  // Set the pins
  CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(50000)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received ");

    if (CAN.packetExtended()) {
      Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    } else {
      Serial.print(" and length ");
      Serial.println(packetSize);

      Serial.print("Data: ");
      while (CAN.available()) {
        uint8_t byteData = CAN.read();
        if (byteData < 0x10) {
          // Add a leading zero for single hex digits
          Serial.print("0");
        }
        Serial.print(byteData, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }

    Serial.println();
  }
}
