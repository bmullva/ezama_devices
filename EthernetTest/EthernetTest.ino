#include <ETH.h>          //For ESP32 Dev Module 
#include <WiFiClient.h>
#include <PubSubClient.h>

// Ethernet settings
//#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
//#define ETH_POWER_PIN   -1
#define ETH_POWER_PIN   16
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define LED_PIN         2

// MQTT server details
const char* mqtt_server = "192.168.4.222";
const int mqtt_port = 1883; // Default MQTT port+

WiFiClient ethClient;
PubSubClient client(ethClient);

// Callback function that gets called when a message is received
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize Ethernet with DHCP
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    Serial.println("Ethernet initialized");

    // Wait for Ethernet to connect
    while (!ETH.linkUp()) {
        Serial.println("Waiting for Ethernet link...");
        delay(1000);
    }
    Serial.println("Ethernet link established");

    // Setup MQTT client
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    // Try to connect to MQTT broker
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("WT32-ETH01")) {
            Serial.println("connected");
            // Once connected, subscribe to the "broadcast" topic
            client.subscribe("broadcast");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void loop() {
    if (!client.connected()) {
        // Attempt to reconnect if disconnected
        reconnect();
    }
    client.loop();
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("WT32-ETH01")) {
            Serial.println("connected");
            // Subscribe after reconnecting
            client.subscribe("broadcast");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
