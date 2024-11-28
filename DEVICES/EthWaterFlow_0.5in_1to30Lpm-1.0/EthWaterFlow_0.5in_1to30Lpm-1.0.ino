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


#include <Filters.h>

//Flow sensor is a 5V device, and returns a 

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Water Flow 0.5in - 30Lpm";
String ver = "1.0";

float lpm = 0;
float gpm = 0;
volatile int pulses = 0;    // count pulses
unsigned long lastMillis = 0; // to track time
float totalLiters = 0;
float totalGallons = 0;
void ICACHE_RAM_ATTR isr() { // ISR to increment count variable when pin goes high
  pulses++;
}

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
  state_json["vG"]        = "gpm,0,30;lpm,0,120";
  //state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  //state_json["pL"]        = "1,12,;1,3,AC";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);
  
  topic = String(device_id) + "/gpm";  
  mqttClient.publish(topic.c_str(), String(gpm).c_str());

  topic = String(device_id) + "/lpm";  
  mqttClient.publish(topic.c_str(), String(lpm).c_str());

  topic = String(device_id) + "/gal";  
  mqttClient.publish(topic.c_str(), String(totalGallons).c_str());

  topic = String(device_id) + "/liter";  
  mqttClient.publish(topic.c_str(), String(totalLiters).c_str());
  
}

// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved

// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
}

// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}

// 6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  topic = String(device_id) + "/" + String("gpm");
  mqttClient.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("lpm");
  mqttClient.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("gal");
  mqttClient.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("liter");
  mqttClient.subscribe(topic.c_str());
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(14), isr, RISING); // Attach ISR to pin's rising edge interrupt
}

// 7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h

  // Calculate flow rate and total volume every second
  if (millis() - lastMillis >= 1000) {
    noInterrupts(); // Disable interrupts
    float frequency = pulses; // Hz
    pulses = 0; // Reset pulse count
    interrupts(); // Enable interrupts

    lpm = frequency / 7.5; // Liters per minute
    gpm = lpm * 0.264172; // Gallons per minute

    float liters = frequency / 7.5 / 60.0; // Total liters in the past second
    float gallons = liters * 0.264172; // Total gallons in the past second

    totalLiters += liters;
    totalGallons += gallons;

    lastMillis = millis();
  }
}
