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

//#include <Filters.h>
//Motion sensor is a 5V device, and returns a 3.3V Signal

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Ethernet RCWL-0516 Motion";
String ver = "1.0";

String onOff = "off";
String topic {};

// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"]      = type_;
  state_json["ver"]       = ver;
  state_json["IP"]        = WiFi.localIP();
  //state_json["vG"]        = "depth,0,20";
  //state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  //state_json["pL"]        = "1,12,;1,3,AC";
  state_json["pS"]        = "0,0,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);

}



// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
}



// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {

  topic = String(device_id)+"/"+String("onOff0");
  mqttClient.subscribe(topic.c_str());

}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  
  pinMode(12, INPUT);
}


//7 MAIN LOOP

void loop() {
  ezama_loop();  //in ezama.h
 
  if (digitalRead(12) == HIGH && onOff == "off") {        
    onOff = "on";
    mqttClient.publish(topic.c_str(), String(onOff).c_str()); 
  }

  if (digitalRead(12) == LOW && onOff == "on") {        
    onOff = "off";
    mqttClient.publish(topic.c_str(), String(onOff).c_str());
  }

  delay(1000);
}
