#include <CD74HC4067.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Mull";
const char* password = "4086794040";

// MQTT Server
const char* mqtt_server = "192.168.4.222";
const int mqtt_port = 1883;  // Default MQTT port

// Define shared control pins for all six MUXes
const int sharedControlPins[4] = {17, 5, 33, 32};

// Define control pins for the 7th MUX (selecting between the six MUXes)
// Here, we'll assume the fourth pin is not used or grounded; adjust if different
const int selectMuxPins[4] = {12, 14, 15, -1};  // -1 assumes the fourth pin is not used

// Common signal pin for all MUXes
const int commonSIGPin = 4;

CD74HC4067 sharedMux(sharedControlPins[0], sharedControlPins[1], sharedControlPins[2], sharedControlPins[3]);
CD74HC4067 selectMux(selectMuxPins[0], selectMuxPins[1], selectMuxPins[2], selectMuxPins[3]);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
    } else {
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  // Set SIG pin as input
  pinMode(commonSIGPin, INPUT_PULLUP);

  reconnect();
  client.publish("debug", "Switch Hub is Up");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  for(int mux = 0; mux < 6; mux++) {
    selectMux.channel(mux);
    delay(1);
    
    for(int channel = 0; channel < 16; channel++) {
      sharedMux.channel(channel);
      delay(1);
      
      int sensorValue = digitalRead(commonSIGPin);  // Changed to digitalRead
      
      if (sensorValue == LOW) {
        // Prepare the message
        char message[50];
        snprintf(message, sizeof(message), "MUX %d, Channel %d is LOW", mux, channel);
        
        // Publish to MQTT topic
        client.publish("debug", message);
      }
    }
  }
  
  delay(1000);
}
