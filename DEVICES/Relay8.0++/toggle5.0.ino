
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// DO NOT TOUCH - GLOBAL CONSTANTS
String ver = "5.0";
String type = "toggleSwitch";
char device_id[9] = "XXXXXXX";

String ssid = "Mull-2.4GHz";
const char* MQTT_SERVER = "192.168.1.222";
char password[32] = "4086794040";
//String ssid = "DCNetwork-00000001";
//const char* MQTT_SERVER = "10.10.1.1";
//char password[32] = "DCNetwork";


//********************
//****   UPDATE   ****
//********************
// INITIALIZE STATE CONSTANTS & VARIABLES
int p14 {};
int p12  {};
int p13  {};
//float pA0  {};
int p14_n1 {};
int p12_n1 {};
int p13_n1 {};
//float pA0_n1 {};
StaticJsonDocument<256> jdoc;


// INITIALIZE GLOBAL MODULES
//***********************************
//****  UPDATE BASED ON MODULES  ****
//*********************************** 
WiFiClient espClient;
PubSubClient client(espClient);


// DO NOT TOUCH
void setup_wifi() {
  delay(5000);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


//*************************************
//****  UPDATE REPORTING AND IDS   ****
//*************************************
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["IP"] = WiFi.localIP();
  p14 = digitalRead(14);
  p12 = digitalRead(12);
  p13 = digitalRead(13);
  //pA0 = analogRead(0);
  
  if(p14 == HIGH) {state_json["p14"] = "off";}
  else {state_json["p14"] = "on";}
  if(p12 == HIGH) {state_json["p12"] = "off";}
  else {state_json["p12"] = "on";}
  if(p13 == HIGH) {state_json["p13"] = "off";}
  else {state_json["p13"] = "on";}
  
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);
}

void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["pins"]= "[p14, p12, p13]";
  state_json["ver"] = ver;
  state_json["action"] = "[on,off]";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}
  
void publish_controls_json(String pin_name, String pin_msg) {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "control";
  state_json["device_id"] = device_id;
  state_json[pin_name] = pin_msg;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// CALLBACK
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // STANDARD PING RETURN
  if(topic=="broadcast"){   // move most of this into the publish state function
    if(messageTemp == "ids") {publish_ids_json();}    
    if(messageTemp == "ping") {publish_reporting_json();}
    if(messageTemp == "restart") {ESP.restart();}
  }
  
    //******************************************
    // UPDATE: ADD ANY ADDITIONAL MESSAGE-BASED ACTIONS HERE
    //****************************************** 

  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("device_id: ");
  Serial.println(String(device_id));
  Serial.print("Eval: ");
  Serial.println(topic==String(device_id));
  
  if(topic==String(device_id)){
      Serial.print("Received message: ");
      Serial.print(messageTemp);
      DeserializationError error = deserializeJson(jdoc, messageTemp);
      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
    }
  Serial.println();
}

// DO NOT TOUCH (Re)Connect ESP8266 to MQTT broker 
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    WiFi.mode(WIFI_STA);
    if (client.connect(device_id)) {
      Serial.print("connected ");
      Serial.print(device_id);  
      Serial.println(".");
      // subscribe to topic
      client.subscribe(device_id);
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


// SETUP AND ASSIGN EEPROM
void setup() { 
  Serial.begin(115200);
    setup_wifi();
    client.setServer(MQTT_SERVER, 1883);
    client.setCallback(callback);
    ArduinoOTA.setHostname(device_id);
    ArduinoOTA.begin();
    
    //***********************************
    //****  UPDATE INITIALIZE PINS   ****
    //***********************************
    pinMode(14, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
    pinMode(13, INPUT_PULLUP);
    //pinMode(pA0, INPUT_PULLUP);
}


// DO NOT TOUCH - Ensures ESP is connected & sends messages to topics
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()) {
    WiFi.mode(WIFI_STA);
    client.connect(device_id);
  }
  
  //**********************************
  // UPDATE: SEND MESSAGES TO TOPICS
  //**********************************
  p14 = digitalRead(14);
  p12  = digitalRead(12);
  p13  = digitalRead(13);
  //pA0  = analogRead(A0);

  if (p14 != p14_n1) {
    if (p14 == HIGH){publish_controls_json("p14","off");}
    if (p14 == LOW){publish_controls_json("p14","on");}
    p14_n1 = p14;
  }
  if (p12 != p12_n1) {
    if (p12 == HIGH){publish_controls_json("p12","off");}
    if (p12 == LOW){publish_controls_json("p12","on");}
    p12_n1 = p12;
  }
  if (p13 != p13_n1) {
    if (p13 == HIGH){publish_controls_json("p13","off");}
    if (p13 == LOW){publish_controls_json("p13","on");}
    p13_n1 = p13;
  }  
  delay(100);
} 
