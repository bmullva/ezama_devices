#include <Ezama11.h>  // For ESP-32 DOIT ESP23
#include <OneWire.h>
#include <DallasTemperature.h>
//This is a 5V device, and returns 5V digital signal.

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "DS18B20 Temperature Sensor";
String ver = "8.1";

#define ONE_WIRE_BUS 14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float t;



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
  state_json["vG"]        = "tempC,-20,100;tempF,-20,200";
  //state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  //state_json["pL"]        = "1,12,;1,3,AC";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);

  topic = String(device_id)+"/tempC";  
  client.publish(topic.c_str(), String(tempC).c_str());

  topic = String(device_id)+"/tempF";  
  client.publish(topic.c_str(), String(tempF).c_str());

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
  String topic {};
  
  topic = String(device_id)+"/"+String("tempC");
  client.subscribe(topic.c_str());

  topic = String(device_id)+"/"+String("tempF");
  client.subscribe(topic.c_str());

}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();

  pinMode(14, INPUT_PULLUP);
  sensors.begin();
}



//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  t = sensors.getTempCByIndex(0);
  delay(1000);
}
