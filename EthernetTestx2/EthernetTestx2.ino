#include <ETH.h>          //For ESP32 Dev Module 
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

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

#define MCP23017_SCL 5
#define MCP23017_SDA 17

//DEFINE EEPROM ADDRESSES
//const int device_id_addr = 222; 
//8 digit (222-229) device_id
const int password_length_addr = 231; // 8 <= len <= 63
// 232-239 NOT USED
const int password_addr = 240; // 8-63 byte (240-302)
unsigned long previousAttemptTime = 0;
const unsigned long reconnectInterval = 5000;
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
char mqtt_ip[16] {};
char network_id[33] {};
char device_id[9] {};
char password[64] {};
int password_length {};
IPAddress remote_ip;
String mcp_found {};

String type_ = "EtherNet Test";
String ver = "10.3";

WiFiClient ethClient;
PubSubClient client(ethClient);

Adafruit_MCP23X17 mcp0;

//MQTT CONNECTION CODE
bool connectToMQTT() {
  // Attempt to connect to the first 3 IP addresses:
  client.setServer(mqtt_ip_1, 1883);
  if (client.connect(device_id)) {
    remote_ip = IPAddress(192, 168, 0, 222);
    return true;  // Connected successfully
  }
  client.setServer(mqtt_ip_2, 1883);
  if (client.connect(device_id)) {
    remote_ip = IPAddress(192, 168, 1, 222);
    return true;  // Connected successfully
  }
    client.setServer(mqtt_ip_3, 1883);
  if (client.connect(device_id)) {
    remote_ip = IPAddress(192, 168, 4, 222);
    return true;  // Connected successfully
  }

  return false;  // Both connection attempts failed
}


void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  state_json["MCP"] = mcp_found;
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}

void receive_controls_json(String topic, String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
}


//CALLBACK
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);

  if (strcmp(topic, "broadcast") == 0) {
    if (messageTemp == "ping") {publish_reporting_json();}
    if (messageTemp == "restart") { 
      delay(10000);  // Delay before restarting
      ESP.restart();
    }
  }
  else if (strcmp(topic, device_id) == 0) {
    if (messageTemp == "restart") {ESP.restart();}
    if (messageTemp == "reset") {
      for (int i = 0; i < 222; i++) {EEPROM.write(i, 255);}
      for (int i = 231; i < 512; i++) {EEPROM.write(i, 255);}
      EEPROM.write(230, 's');
      EEPROM.commit();
      ESP.restart();
    }
  }
  else if (strcmp(topic, "password") == 0) {
    int password_length = messageTemp.length();
    if (password_length > 0 && password_length <= 64) {  // Ensure password is within bounds
      EEPROM.write(password_length_addr, password_length);
      for (int i = 0; i < password_length; i++) {
        EEPROM.write(password_addr + i, messageTemp[i]);
      }
      EEPROM.commit();
      ESP.restart();
    }
    else {
      Serial.println("Invalid password length.");
    }
  }
  else {
    receive_controls_json(topic, messageTemp);
  }
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
      device_id[i] = char(EEPROM.read(222 + i));
    }

    // Initialize Ethernet with DHCP
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    Serial.println("Ethernet initialized");

    // Wait for Ethernet to connect
    while (!ETH.linkUp()) {
        Serial.println("Waiting for Ethernet link...");
        delay(1000);
    }
    Serial.println("Ethernet link established");

    // Setup I2C communication for MCP23017
    Wire.begin(MCP23017_SDA, MCP23017_SCL);
    delay(100); // Optional stabilization delay
    bool mcp_initialized = false;
    String attempt_message = "";
    int attempt {};
    for (attempt = 0; attempt < 10 && !mcp_initialized; attempt++) {
        if (mcp0.begin_I2C(0x20)) {
            mcp_initialized = true;
            attempt_message = "MCP FOUND on attempt " + String(attempt + 1);
            break; // Stop trying once initialized
        } else {
            attempt_message = "MCP NOT FOUND after attempt " + String(attempt + 1);
            delay(500); // Small delay before next attempt
        }
    }

    mcp_found = mcp_initialized ? "MCP FOUND on attempt " + String(attempt + 1) : attempt_message;

    // Configure MCP23017 pins here if needed, e.g., setting pins as input or output
    // Example:
    // mcp.pinMode(0, OUTPUT); // Set pin 0 of MCP23017 as output
    // mcp.pinMode(1, INPUT);  // Set pin 1 of MCP23017 as input

    //CONNECT TO MQTT
    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.subscribe("password");
        client.setCallback(callback);
        Serial.println("MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }
}


void setup2() {
    Serial.begin(115200);
    delay(1000);

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
      device_id[i] = char(EEPROM.read(222 + i));
    }

    // Initialize Ethernet with DHCP
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    Serial.println("Ethernet initialized");

    // Wait for Ethernet to connect
    while (!ETH.linkUp()) {
        Serial.println("Waiting for Ethernet link...");
        delay(1000);
    }
    Serial.println("Ethernet link established");

  //CONNECT TO MQTT
  if (connectToMQTT()) {
    client.subscribe(device_id);
    client.subscribe("broadcast");
    client.subscribe("password");
    client.setCallback(callback);
    Serial.println("MQTT connected");
  } else {
    Serial.println("MQTT connection failed");
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
