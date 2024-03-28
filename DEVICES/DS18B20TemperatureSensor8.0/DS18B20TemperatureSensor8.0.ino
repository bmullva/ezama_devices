#include <Ezama2.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "DS18B20 Temperature Sensor";
String ver = "8.0";

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
  state_json["type"] = type;
  state_json["ver"] = ver;
  state_json["IP"] = WiFi.localIP();
  state_json["temp_C"] = t;
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = device_id+"/C";  
  client.publish(topic.c_str(), tempC);
  
  topic = device_id+"/F";  
  client.publish(topic.c_str(), tempF);
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  
  state_json["vGage"] = "[[\"C\", -20, 100], [\"F\", -20, 200]]";
  //state_json["vLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pPwtch"] = "[['start#', 'end#', 'name']]";
  
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {

}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(14, INPUT_PULLUP);
  sensors.begin();

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  t = sensors.getTempCByIndex(0);
  delay(1000);
  
}
