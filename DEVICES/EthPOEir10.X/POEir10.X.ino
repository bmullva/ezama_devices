#include <ETH.h>          //For ESP32 Dev Module
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

//MAYBE OTA?

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "POE Momentary";
String ver = "10.3";

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   -1
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define LED_PIN         2

WiFiClient ethClient;
PubSubClient mqttClient(ethClient);

//const char* mqtt_server = "192.168.4.222";
const char* mqtt_server {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
char device_id[9] {};
const int mqtt_port = 1883;
const char* mqtt_topic_subscribe = "broadcast";
const char* mqtt_topic_publish = "reporting";
static bool eth_connected = false;
//const int device_id_addr = 222; 
//8 digit (222-229) device_id
const int password_length_addr = 231; // 8 <= len <= 63
// 232-239 NOT USED
const int password_addr = 240; // 8-63 byte (240-302)

int d_pin_reading [4]         = {HIGH, HIGH, HIGH, HIGH};
int d_pin_n1_reading [4]      = {HIGH, HIGH, HIGH, HIGH};
unsigned long startMillis [4] = {0, 0, 0, 0};
int clk [4]                   = {0, 0, 0, 0};
int rel [4]                   = {0, 0, 0, 0};
int mom_pins [4] = {4, 5, 12, 14};

// Define enums for different states
enum ButtonState {
  OFF,
  ON
};
ButtonState timer[4]          = {OFF, OFF, OFF, OFF};

//Need to install the following:


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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
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


//MAYBE? OTA?
// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);
}

// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
}

// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
// Only these will be published: "on", "off", "dim", "brighten", "heat", "cool"

void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  mqttClient.publish(topic.c_str(), pin_msg.c_str());
}


//6 SETUP (pins)
void specific_connect() {
}

void findMQTTserver() {
  mqttClient.setServer(mqtt_ip_1, mqtt_port);
  Serial.print("I have tried mqtt1");
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_1;
    return;  // Connected successfully
  }
  mqttClient.setServer(mqtt_ip_2, mqtt_port);
  Serial.print("I have tried mqtt2");
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_2;
    return;  // Connected successfully
  }
  mqttClient.setServer(mqtt_ip_3, mqtt_port);
  Serial.print("I have tried mqtt3");
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_3;
    Serial.print("I have connected to the MQTT client");
    return;  // Connected successfully
  }
}

void setup() {
  long randomDelay = random(0, 10001);
  delay(randomDelay);
  
  //pinMode(4, INPUT_PULLUP);
  //pinMode(5, INPUT_PULLUP);
  //pinMode(12, INPUT_PULLUP);
  //pinMode(14, INPUT_PULLUP);
  //pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

  EEPROM.begin(512);
  for (int i = 0; i < 8; i++) {
    device_id[i] = char(EEPROM.read(222 + i));
    }
  Serial.println("ESP32-Stick begin\n");
  WiFi.onEvent(WiFiEvent);
  Serial.printf("%d\n\r", ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE));
  findMQTTserver();
  mqttClient.subscribe(mqtt_topic_subscribe);
  //mqttClient.setServer(mqtt_server, mqtt_port);
}


//7 MAIN LOOP#####
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

//unsigned long previousMillis = 0;
//const long interval = 5000; // 5 seconds

void loop() {
  if (!mqttClient.connected()) {
      reconnect();
  }
  mqttClient.loop();
  
  if (eth_connected && IrReceiver.decode()) {
    if (!IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      uint8_t command = IrReceiver.decodedIRData.command; // Store command in a variable

      switch (command) {
        case 0x16:
          Serial.println("star");
          publish_controls("0","star");
          break;
        case 0xD:
          Serial.println("hash");
          publish_controls("0","hash");
          break;
        case 0x45:
          Serial.println("10");
          publish_controls("0","10");
          break;
        case 0x46:
          Serial.println("20");
          publish_controls("0","20");
          break;
        case 0x47:
          Serial.println("30");
          publish_controls("0","30");
          break;
        case 0x44:
          Serial.println("40");
          publish_controls("0","40");
          break;
        case 0x40:
          Serial.println("50");
          publish_controls("0","50");
          break;
        case 0x43:
          Serial.println("60");
          publish_controls("0","60");
          break;      
        case 0x7:
          Serial.println("70");
          publish_controls("0","70");
          break;
        case 0x15:
          Serial.println("80");
          publish_controls("0","80");
          break;
        case 0x9:
          Serial.println("90");
          publish_controls("0","90");
          break;
        case 0x19:
          Serial.println("100");
          publish_controls("0","100");
          break;
        case 0x18:
          Serial.println("brighten");
          publish_controls("0","brighten");
          break;
        case 0x52:
          Serial.println("dim");
          publish_controls("0","dim");
          break;
        case 0x8:
          Serial.println("cool");
          publish_controls("0","cool");
          break;
        case 0x5A:
          Serial.println("heat");
          publish_controls("0","heat");
          break;
        case 0x1C:
          Serial.println("release");
          publish_controls("0","release");
          break;
        
        default:
          Serial.println(command, HEX);
          break;
      }
    }

    // Resume receiver to get the next signal
    IrReceiver.resume();
  }
}
