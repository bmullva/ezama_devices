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

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Ethernet Momentary";
String ver = "1.0";

int d_pin_reading [4]         = {HIGH, HIGH, HIGH, HIGH};
int d_pin_n1_reading [4]      = {HIGH, HIGH, HIGH, HIGH};
unsigned long startMillis [4] = {0, 0, 0, 0};
int clk [4]                   = {0, 0, 0, 0};
int rel [4]                   = {0, 0, 0, 0};
int mom_pins [4] = {4, 13, 12, 14};

// Define enums for different states
enum ButtonState {
  OFF,
  ON
};
ButtonState timer[4]          = {OFF, OFF, OFF, OFF};


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
  ezama_setup();  //in ezama.h
  Serial.begin(115200);
  
  pinMode(4, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
}


//7 MAIN LOOP#####
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

//unsigned long previousMillis = 0;
//const long interval = 5000; // 5 seconds

void loop() {
  ezama_loop();
  
  //Serial.println(eth_connected);
  if (eth_connected) {

  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(mom_pins[i]);
  }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 4; i++) {  
    
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
          if (i == 0) {publish_controls("0", "release");}
          if (i == 1) {publish_controls("0", "release");}
          if (i == 2) {publish_controls("1", "release");}
          if (i == 3) {publish_controls("1", "release");}
    }
    
    // Check for a button press (transition from HIGH to LOW) and start a timer
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == OFF) {
      startMillis[i] = millis();
      timer[i] = ON;
    }
  
    // Check for a button release (transition from LOW to HIGH)
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == ON) {
      rel[i] += 1;
    }

    // Check for a button press (transition from HIGH to LOW)
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == ON) {
      clk[i] += 1;
    }
    
    // Check for timer expiration
    if (millis() - startMillis[i] > 500) {
      if (timer[i] == ON) {
        if (clk[i] == 1 && rel[i] == 0) {                     //hold
          if (i == 0) {publish_controls("0", "dim");}
          if (i == 1) {publish_controls("0", "brighten");}
          if (i == 2) {publish_controls("1", "dim");}
          if (i == 3) {publish_controls("1", "brighten");}
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          if (i == 0) {publish_controls("0", "off");}
          if (i == 1) {publish_controls("0", "on");}
          if (i == 2) {publish_controls("1", "off");}
          if (i == 3) {publish_controls("1", "on");}   
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("0", "cool");}
          if (i == 1) {publish_controls("0", "heat");}
          if (i == 2) {publish_controls("1", "cool");}
          if (i == 3) {publish_controls("1", "heat");} 
        } //else if (clk[i] == 2 && rel[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("0", "??");}
         // if (i == 1) {publish_controls("0", "??");}
         // if (i == 2) {publish_controls("1", "??");}
         // if (i == 3) {publish_controls("1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk[i] = 0;
      rel[i] = 0;
      timer[i] = OFF;
    }
  
    // Update the previous reading for the next iteration
    d_pin_n1_reading[i] = d_pin_reading[i];
  }

  delay(100);
    
  }
}
