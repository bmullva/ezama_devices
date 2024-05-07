#include <Wire.h>
#include <ArduinoJson.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
}

void loop() {
  String topic = "temperature";
  String message = "25C";

  StaticJsonDocument<200> doc;
  doc["topic"] = topic;
  doc["message"] = message;

  Wire.beginTransmission(8); // Address of the second ESP32
  serializeJson(doc, Wire);
  Wire.endTransmission();

  delay(5000);
}
