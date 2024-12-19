#include <ETH.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// Ethernet settings
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
Adafruit_MCP23X17 mcp4;

const char* mqtt_server = "192.168.4.222";  // Replace with your MQTT server IP
const char* mqtt_topic = "debug/mcp_status";  // Topic for debugging messages

bool mcpInitialized = false; // Flag to check if MCP is initialized
unsigned long lastMCPInitAttempt = 0;

void setup() {
    Serial.begin(115200); // For debugging

    // Ethernet setup
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    while (!ETH.linkUp()) delay(1000);

    client.setServer(mqtt_server, 1883);  // MQTT broker port
    Wire.begin(MCP23017_SDA, MCP23017_SCL);

    // Setup reset pin
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);  // Pull reset pin low to reset MCP23017
    delay(100); 
    digitalWrite(RESET_PIN, HIGH); // Keep reset high by default

    // Attempt to connect to MQTT in setup for initial connection
    connectMQTT();
}

void loop() {
    if (!client.connected()) {
        connectMQTT(); // Try to reconnect if disconnected
    }
    client.loop(); // Handle MQTT communication

    // Check and initialize MCP23017 if not already done
    delay(1000);
    if (!mcpInitialized) {
        if (millis() - lastMCPInitAttempt > 5000) { // Try every 5 seconds if failed
            if (mcp0.begin_I2C(0x24)) {
                mcpInitialized = true;
                client.publish(mqtt_topic, "MCP23017 Initialized");
            } else {
                client.publish(mqtt_topic, "MCP23017 Init Failed, Attempting Reset");
                resetMCP23017();
                lastMCPInitAttempt = millis();
            }
        }
    } else {
        // If MCP is initialized, proceed with normal operations
        String i2cStatus = scanI2C();
        client.publish(mqtt_topic, i2cStatus.c_str());

        // Communicate with slaves
        String slaveResponses = "";
        communicateWithSlave(0x20, "Hello 20", slaveResponses);
        communicateWithSlave(0x21, "Hello 21", slaveResponses);
        communicateWithSlave(0x22, "Hello 22", slaveResponses);
        communicateWithSlave(0x24, "Hello 24", slaveResponses);
        
        if (!slaveResponses.isEmpty()) {
            client.publish(mqtt_topic, slaveResponses.c_str());
        }
    }

    delay(3000); // Toggle every 3 seconds, adjust as needed
}

void communicateWithSlave(uint8_t address, const char* message, String& response) {
    Wire.beginTransmission(address);
    Wire.write(message);
    Wire.endTransmission();
    
    delay(10); // Give slave some time to prepare response
    
    Wire.requestFrom(address, 32); // Request up to 32 bytes from slave
    if(Wire.available()) {
        response += "Slave 0x" + String(address, HEX) + ": " + Wire.readString() + "; ";
    }
}


bool initializeMCPs() {
    mcp4.begin_I2C(0x24);
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

void resetMCP23017() {
    digitalWrite(RESET_PIN, LOW);  // Pull reset pin low to reset MCP23017
    delay(100);                     // Keep reset low for 10ms; adjust based on needs
    digitalWrite(RESET_PIN, HIGH); // Release reset
}

String scanI2C() {
    String result = "";
    byte error, address;
    int nDevices = 0;

    for (address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            result += "Dev@0x" + String(address, HEX) + " ";
            nDevices++;
        }
    }

    if (nDevices == 0) {
        result = "NoI2C";
    } else {
        result.trim();
    }

    return result;
}
