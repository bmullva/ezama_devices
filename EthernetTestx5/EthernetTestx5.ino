#include <ETH.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// Ethernet settings (use your existing setup)
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define MCP23017_SDA 17
#define MCP23017_SCL 5

WiFiClient ethClient;
PubSubClient client(ethClient);

Adafruit_MCP23X17 mcp0;

const char* mqtt_server = "192.168.4.222";  // Replace with your MQTT server IP
const char* mqtt_topic = "debug/mcp_status";  // Topic for debugging messages

bool mcpInitialized = false; // Flag to check if MCP is initialized

void setup() {
    Serial.begin(115200); // For debugging, though you can't see it

    // Ethernet setup
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    while (!ETH.linkUp()) delay(1000);

    client.setServer(mqtt_server, 1883);  // MQTT broker port
    Wire.begin(MCP23017_SDA, MCP23017_SCL);

    connectMQTT();
}

void loop() {
    if (!client.connected()) {
        connectMQTT(); // Try to reconnect if disconnected
    }
    client.loop(); // Handle MQTT communication

    // Check and initialize MCP23017 if not already done
    if (!mcpInitialized) {
        if (mcp0.begin_I2C(0x20)) {
            mcpInitialized = true;
            for (int i = 0; i < 16; i++) {
                mcp0.pinMode(i, INPUT_PULLUP); // Set all pins to input with pull-ups
            }
            client.publish(mqtt_topic, "MCP23017 Initialized as Input");
        } else {
            static unsigned long lastMCPInitAttempt = 0;
            if (millis() - lastMCPInitAttempt > 5000) { // Try every 5 seconds if failed
                client.publish(mqtt_topic, "MCP23017 Init Failed, Retrying");
                lastMCPInitAttempt = millis();
            }
            return; // Skip further execution if MCP not initialized
        }
    }

    // Read from MCP23017 and report via MQTT
    for (int i = 0; i < 16; i++) {
        String state = "Pin " + String(i) + ": " + String(mcp0.digitalRead(i));
        client.publish(mqtt_topic, state.c_str());
        delay(500); // Small delay to not flood MQTT
    }

    delay(1000); // Loop every second
}

void connectMQTT() {
    while (!client.connected()) {
        if (client.connect("MCP23017Test")) {
            client.publish(mqtt_topic, "Connected to MQTT");
        } else {
            delay(5000);
        }
    }
}
