#include <ETH.h>          // For ESP32 Dev Module
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// Constants and variables
const String type_ = "Ethernet Switch Hub";
const String ver = "2.1";

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   -1
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define LED_PIN         2

// Network and MQTT settings
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
const int mqtt_port = 1883;
const char* mqtt_topic_subscribe = "broadcast";
const char* mqtt_topic_publish = "reporting";
static bool eth_connected = false;

// Array to hold multiple MCP23X17 instances
Adafruit_MCP23X17 mcps[8];

// IP addresses for MQTT server
char* mqtt_servers[] = {"192.168.0.222", "192.168.1.222", "192.168.4.222"};
const char* mqtt_server = nullptr;
char device_id[9] = {};

// EEPROM addresses for device configuration
const int device_id_addr = 222; 
const int password_length_addr = 231; 
const int password_addr = 240;

// Define enum for button state
enum ButtonState {OFF, ON};

// Arrays for managing button states, timers, and readings
int mcp_readings[8][12] = { {HIGH} };
int mcp_n1_readings[8][12] = { {HIGH} };
unsigned long startMillis[8][12] = { {0} };
int clk[8][12] = { {0} };
int rel[8][12] = { {0} };
ButtonState timers[8][12] = { {OFF} };

const int mom_pins[12] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13};

// Function prototypes
void WiFiEvent(WiFiEvent_t event);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnect();
void publish_reporting_json();
void receive_controls_json(String topic, String messageTemp);
void publish_controls(String switch_num, String pin_msg);
void findMQTTserver();

// WiFi event handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      mqttClient.setServer(mqtt_server, mqtt_port);
      mqttClient.setCallback(mqttCallback);
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(messageTemp);

  if (strcmp(topic, "broadcast") == 0) {
    if (messageTemp == "ping") { publish_reporting_json(); }
    else if (messageTemp == "restart") { 
      delay(10000);  
      ESP.restart();
    }
  } else if (strcmp(topic, device_id) == 0) {
    if (messageTemp == "restart") { ESP.restart(); }
    else if (messageTemp == "reset") {
      for (int i = 0; i < 222; i++) { EEPROM.write(i, 255); }
      for (int i = 231; i < 512; i++) { EEPROM.write(i, 255); }
      EEPROM.write(230, 's');
      EEPROM.commit();
      ESP.restart();
    }
  } else if (strcmp(topic, "password") == 0) {
    int password_length = messageTemp.length();
    if (password_length > 0 && password_length <= 64) {  
      EEPROM.write(password_length_addr, password_length);
      for (int i = 0; i < password_length; i++) {
        EEPROM.write(password_addr + i, messageTemp[i]);
      }
      EEPROM.commit();
      ESP.restart();
    } else {
      Serial.println("Invalid password length.");
    }
  } else {
    receive_controls_json(topic, messageTemp);
  }
}

// MQTT reconnect function
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Function to publish device status
void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  char sj[1024];
  String topic = "reporting";

  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP().toString();
  state_json["pS"] = "0,23,";

  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);
}

// Function to handle controls received
void receive_controls_json(String topic, String messageTemp) {
  if(messageTemp == "restart") { ESP.restart(); }
}

// Function to publish control messages
void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  mqttClient.publish(topic.c_str(), pin_msg.c_str());
}

// Function to find MQTT server
void findMQTTserver() {
  for (int i = 0; i < sizeof(mqtt_servers) / sizeof(mqtt_servers[0]); i++) {
    mqttClient.setServer(mqtt_servers[i], mqtt_port);
    if (mqttClient.connect(device_id)) {
      mqtt_server = mqtt_servers[i];
      return;
    }
  }
}

// Setup function
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  
  // Load device ID from EEPROM
  for (int i = 0; i < 8; i++) {
    device_id[i] = char(EEPROM.read(device_id_addr + i));
  }
  device_id[8] = '\0'; // Null terminate the string

  Serial.println("Ether Switch Begin\n");
  WiFi.onEvent(WiFiEvent);
  Serial.printf("%d\n\r", ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE));
  
  // Initialize MCP23X17 chips
  for (int i = 0; i < 8; i++) {
    if (!mcps[i].begin_I2C(0x20 + i)) {
      Serial.print("Error initializing MCP23017x");
      Serial.println(i);
      while (1); // Loop forever if there's an error
    }
    for (int pin : mom_pins) {
      mcps[i].pinMode(pin, INPUT_PULLUP);
    }
  }

  findMQTTserver();
  mqttClient.subscribe(mqtt_topic_subscribe);
}

// Main loop
void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  if (eth_connected) {
    for (int mcp = 0; mcp < 8; mcp++) {
      for (int i = 0; i < 12; i++) {
        mcp_readings[mcp][i] = mcps[mcp].digitalRead(mom_pins[i]);
        handleButtonEvent(mcp, i);
      }
    }
  }
  delay(100);
}

// Handle button events for one MCP and one button
void handleButtonEvent(int mcp, int button) {
  int base_num = mcp * 3; // Each MCP controls 3 switch pairs
  String switch_num = String(base_num + (button / 2)) + "/" + String(button % 2);

  if (mcp_n1_readings[mcp][button] == LOW && mcp_readings[mcp][button] == HIGH && timers[mcp][button] == OFF) {
    publish_controls(switch_num, "release");
  } else if (mcp_n1_readings[mcp][button] == HIGH && mcp_readings[mcp][button] == LOW && timers[mcp][button] == OFF) {
    startMillis[mcp][button] = millis();
    timers[mcp][button] = ON;
  } else if (mcp_n1_readings[mcp][button] == LOW && mcp_readings[mcp][button] == HIGH && timers[mcp][button] == ON) {
    rel[mcp][button] += 1;
  } else if (mcp_n1_readings[mcp][button] == HIGH && mcp_readings[mcp][button] == LOW && timers[mcp][button] == ON) {
    clk[mcp][button] += 1;
  }

  if (millis() - startMillis[mcp][button] > 500 && timers[mcp][button] == ON) {
    handleTimerExpiration(mcp, button, switch_num);
    // Reset variables and timer
    clk[mcp][button] = 0;
    rel[mcp][button] = 0;
    timers[mcp][button] = OFF;
  }

  mcp_n1_readings[mcp][button] = mcp_readings[mcp][button];
}

// Handle timer expiration events
void handleTimerExpiration(int mcp, int button, String switch_num) {
  if (clk[mcp][button] == 1 && rel[mcp][button] == 0) { // hold
    publish_controls(switch_num, button % 2 == 0 ? "dim" : "brighten");
  } else if (clk[mcp][button] == 1 && rel[mcp][button] == 1) { // click
    publish_controls(switch_num, button % 2 == 0 ? "off" : "on");
  } else if (clk[mcp][button] == 2 && rel[mcp][button] == 1) { // click-hold
    publish_controls(switch_num, button % 2 == 0 ? "cool" : "heat");
  }
}
