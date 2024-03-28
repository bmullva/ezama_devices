#include <WiFi.h>
#include <PubSubClient.h>

// Replace with your Wi-Fi credentials
const char *ssid = "Ezama";
const char *password = "EzamaEzama";

// Replace with your MQTT broker IP address
const char *mqttServer = "192.168.0.222";

// Replace with your MQTT credentials
//const char *mqttUsername = "your_username";  // If no authentication, leave it empty
//const char *mqttPassword = "your_password";  // If no authentication, leave it empty

const char *mqttTopic = "reporting";
const char *messageToSend = "Message";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqttServer, 1883); // 1883 is the default MQTT port

  // Uncomment the following line if your MQTT broker requires authentication
  // client.setCredentials(mqttUsername, mqttPassword);

  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  // Send a message to the MQTT topic
  if (client.publish(mqttTopic, messageToSend)) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Failed to send message");
  }

  // Wait for 5 seconds before sending the next message
  delay(5000);
}
