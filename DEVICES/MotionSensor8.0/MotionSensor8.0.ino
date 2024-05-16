#include <Ezama12.h>  // For ESP8266
#include <Filters.h>
//Motion sensor is a 5V device, and returns a 3.3V Signal

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "RCWL-0516 Motion Sensor";
String ver = "8.0";

String onOff {};
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
  client.publish(topic.c_str(), sj);

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

  topic = String(device_id)+"/"+String("onOff");
  client.subscribe(topic.c_str());

}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  
  pinMode(14, INPUT);
}


//7 MAIN LOOP

void loop() {
  ezama_loop();  //in ezama.h
 
  if (digitalRead(14) == HIGH && onOff == "off") {        
    onOff = "on";
    client.publish(topic.c_str(), String(onOff).c_str()); 
  }

  if (digitalRead(14) == LOW && onOff == "on") {        
    onOff = "off";
    client.publish(topic.c_str(), String(onOff).c_str());
  }
  
  delay(1000);
}
