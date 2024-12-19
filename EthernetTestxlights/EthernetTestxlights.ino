#include <ETH.h>          //For WT32-ETH01
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

// EEPROM addresses
const int password_length_addr = 231; // 8 <= len <= 63
const int password_addr = 240; // 8-63 byte (240-302)

// Global variables
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};
char device_id[9] {};
char password[64] {};


WiFiClient ethClient;
PubSubClient client(ethClient);
Adafruit_MCP23X17 mcp0;

// FUNCTION PROTOTYPES
bool connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void setupEthernet();

void initializeMCP();
void resetMCP23017();
void publish_reporting_json();
void receive_controls_json(String topic, String message);
void publish_controls(String switch_num, String pin_msg);
void specific_connect();
void custom_setup();
void custom_loop();

void setup() {
    Serial.begin(115200);
    delay(1000);

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    setupEthernet();
    
    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.subscribe("password");
        client.setCallback(callback);
        Serial.println("MQTT connected");
        client.publish("debug", "MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }

    client.publish("debug", "I am at custom_setup");
    custom_setup();
    client.publish("debug", "I have completed custom_setup");
    client.publish("debug", "I am at specific_connect");
    specific_connect();
    client.publish("debug", "I have completed specific_connect");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    custom_loop();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(device_id)) {
            Serial.println("connected");
            client.subscribe("broadcast");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

bool connectToMQTT() {
    for (int i = 0; i < sizeof(mqtt_ips) / sizeof(mqtt_ips[0]); i++) {
        client.setServer(mqtt_ips[i], 1883);
        if (client.connect(device_id)) {
            return true;
        }
    }
    return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(" Message: ");
    Serial.println(message);

    if (strcmp(topic, "broadcast") == 0) {
        if (message == "ping") {
          publish_reporting_json();
        }
        else if (message == "restart") {
            delay(10000);
            ESP.restart();
        }
    } else if (strcmp(topic, device_id) == 0) {
        if (message == "restart") ESP.restart();
        else if (message == "reset") {
            for (int i = 0; i < 222; i++) EEPROM.write(i, 255);
            for (int i = 231; i < 512; i++) EEPROM.write(i, 255);
            EEPROM.write(230, 's');
            EEPROM.commit();
            ESP.restart();
        }
    } else if (strcmp(topic, "password") == 0) {
        int password_length = message.length();
        if (password_length > 0 && password_length <= 64) {
            EEPROM.write(password_length_addr, password_length);
            for (int i = 0; i < password_length; i++) {
                EEPROM.write(password_addr + i, message[i]);
            }
            EEPROM.commit();
            ESP.restart();
        } else {
            Serial.println("Invalid password length.");
        }
    } else {
        receive_controls_json(topic, message);
        client.publish("debug", "Sent to receive controls json");
    }
}

void setupEthernet() {
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    Serial.println("Ethernet initialized");

    while (!ETH.linkUp()) {
        Serial.println("Waiting for Ethernet link...");
        delay(1000);
    }
    Serial.println("Ethernet link established");
}
