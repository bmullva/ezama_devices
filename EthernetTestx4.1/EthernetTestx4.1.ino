#include <ETH.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define MCP23017_SDA 17
#define MCP23017_SCL 5
#define LED_PIN 8
#define RESET_PIN 32  // GPIO32 connected to MCP23017 reset

WiFiClient ethClient;
PubSubClient client(ethClient);
Adafruit_MCP23X17 mcp0;

const char* mqtt_server = "192.168.4.222";
const char* mqtt_topic = "mcp_status";  // Simplified topic name

bool mcpInitialized = false;

void setup() {
    // Ethernet setup
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    while (!ETH.linkUp()) delay(1000);

    client.setServer(mqtt_server, 1883);
    Wire.begin(MCP23017_SDA, MCP23017_SCL);

    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, HIGH); // Keep reset high by default
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT(); 
    }
    client.loop();

    if (!mcpInitialized) {
        initializeMCP();
    } else {
        // Normal operations
        static bool ledState = LOW;
        ledState = !ledState;
        mcp0.digitalWrite(LED_PIN, ledState);
    }

    delay(3000); // Toggle every 3 seconds
}

void reconnectMQTT() {
    while (!client.connected()) {
        if (client.connect("MCP23017")) {
            break; // No need to publish here as it's not debug info
        } else {
            delay(5000);
        }
    }
}

void initializeMCP() {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) {
        if (mcp0.begin_I2C(0x20)) {
            mcpInitialized = true;
            mcp0.pinMode(LED_PIN, OUTPUT); 
        } else {
            resetMCP23017();
        }
        lastAttempt = millis();
    }
}

void resetMCP23017() {
    digitalWrite(RESET_PIN, LOW); 
    delay(10);                     
    digitalWrite(RESET_PIN, HIGH);
}
