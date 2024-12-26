#include <Ezama12.h> // For WEMOS D1 R2 & mini


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Test WiFi";
String ver = "1.0";

String mac {};

// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = WiFi.localIP();
  state_json["MAC"] = WiFi.macAddress();
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
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
}


//6 SETUP (pins)
void specific_connect() {
  
}

void setup() { 
  ezama_setup();  //in ezama8.h
}


//7 MAIN LOOP
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

void loop() {
  ezama_loop();  // Call a function named ezama_loop from ezama.h

  delay(100);
}
