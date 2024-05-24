#include <Ezama12.h>  // For ESP8266, D1 R2 & mini
#include <Filters.h>

//Flow sensor is a 5V device, and returns a 

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Water Flow 0.5in - 30Lpm";
String ver = "8.3";

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
  client.publish(topic.c_str(), sj);
  
  topic = String(device_id) + "/gpm";  
  client.publish(topic.c_str(), String(gpm).c_str());

  topic = String(device_id) + "/lpm";  
  client.publish(topic.c_str(), String(lpm).c_str());

  topic = String(device_id) + "/gal";  
  client.publish(topic.c_str(), String(totalGallons).c_str());

  topic = String(device_id) + "/liter";  
  client.publish(topic.c_str(), String(totalLiters).c_str());
  
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
  client.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("lpm");
  client.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("gal");
  client.subscribe(topic.c_str());

  topic = String(device_id) + "/" + String("liter");
  client.subscribe(topic.c_str());
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
