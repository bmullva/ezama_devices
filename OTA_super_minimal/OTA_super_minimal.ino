#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "Ezama";
const char* password = "EzamaNet";

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
}
