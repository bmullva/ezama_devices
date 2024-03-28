// DO NOT TOUCH
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(network_id);
  WiFi.mode(WIFI_STA);
  WiFi.begin(network_id, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 240) {  //try for 240 half-seconds is two minutes
      EEPROM.write(setting_addr, 's');
      EEPROM.commit();
      ESP.restart();
    }
    i += 1;
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}
