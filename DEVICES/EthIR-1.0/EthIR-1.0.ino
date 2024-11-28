#include <ETH.h>          //For WT32-ETH01
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
const char* mqtt_server {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
char device_id[9] = {};
const int mqtt_port = 1883;
const char* mqtt_topic_subscribe = "broadcast";
const char* mqtt_topic_publish = "reporting";
static bool eth_connected = false;
//const int device_id_addr = 222; 
//8 digit (222-229) device_id
const int password_length_addr = 231; // 8 <= len <= 63
// 232-239 NOT USED
const int password_addr = 240; // 8-63 byte (240-302)

#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Ethernet IR";
String ver = "1.0";

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


void setup() {
  ezama_setup();  //in ezama.ino
  Serial.begin(115200);
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
}


//7 MAIN LOOP#####
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

//unsigned long previousMillis = 0;
//const long interval = 5000; // 5 seconds

void loop() {
  ezama_loop();  //in ezama.ino
  
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
