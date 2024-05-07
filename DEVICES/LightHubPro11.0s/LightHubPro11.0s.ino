#include <Wire.h>
#include <ArduinoJson.h>

void setup() {
  Wire.begin(8);                // Address of this ESP32
  Wire.onReceive(receiveEvent); // Register receive event
  Serial.begin(115200);
}

void loop() {
  delay(100);
}

void receiveEvent(int numBytes) {
  StaticJsonDocument<200> doc;
  while (Wire.available()) {
    deserializeJson(doc, Wire);
  }
  String topic = doc["topic"];
  String message = doc["message"];

  Serial.print("Received Topic: ");
  Serial.println(topic);
  Serial.print("Received Message: ");
  Serial.println(message);
}
