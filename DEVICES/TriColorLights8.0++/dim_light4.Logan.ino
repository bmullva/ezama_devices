#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <ArduinoJson.h>

// DO NOT TOUCH - GLOBAL CONSTANTS
String ssid = "DCNetwork-00000001";
String password= "DCNetwork";
const char* MQTT_SERVER = "10.10.1.1";
const char*  device_id="loganled";


// INITIALIZE GLOBAL MODULES
//***********************************
//****  UPDATE BASED ON MODULES  ****
//*********************************** 
WiFiClient espClient;
PubSubClient client(espClient);


// DO NOT TOUCH
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
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

  if(topic=="led"){
    if(messageTemp == "red on") {
      digitalWrite(14, HIGH);
      Serial.print("red on");   
    }
    if(messageTemp == "blue on") {
      digitalWrite(12,HIGH);
      Serial.print("blue on");
      }   
    if(messageTemp == "green on") {
      digitalWrite(15,HIGH);
      Serial.print("green on");
      }   
    if(messageTemp == "red off") {
      digitalWrite(14,LOW);
      Serial.print("red off");
      }   
    if(messageTemp == "blue off") {
      digitalWrite(12,LOW);
      Serial.print("blue off");
      }   
    if(messageTemp == "green off") {
      digitalWrite(15,LOW);
      Serial.print("green off");
      }   
}
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
      //client.subscribe(device_id);
//*****************************
//**** SUBSCRIBE TO TOPICS ****
//*****************************
		  client.subscribe("led");  
    } 
	 else {
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
    pinMode(14, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(15, OUTPUT);
}


// DO NOT TOUCH - Ensures ESP is connected & sends messages to topics
void loop() {
  
  ArduinoOTA.handle();
  
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()) {
    WiFi.mode(WIFI_STA);
    client.connect(device_id);
  }
}
