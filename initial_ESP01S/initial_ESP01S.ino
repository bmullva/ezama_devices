#include <Ezama10_1.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "Initial";
String ver = "10.1";
int addr = 222;
char id[] = "0000005J";


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
  //state_json["pS"]= "0,3,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {

}


// 5 PUBLISH AND SEND CONTROLS (publish_controls only if controller module)
void publish_controls(String switch_num, String pin_msg) {

}


//6 SETUP (pins)
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  for(int i=0;i<222;i++) {
    EEPROM.write(i, 255);
  }
  for(int i=231;i<512;i++) {
    EEPROM.write(i, 255);
  }
  
  for(int i=0;i<8;i++) {EEPROM.write(addr+i, id[i]);}
  EEPROM.write(230, 's');
  EEPROM.commit();

  digitalWrite(2, LOW);
  ezama_setup();
}


//7 MAIN LOOP
//These are the messages that are being sent: "hold", "click", "click-hold", "dbl-click", "release"

void loop() {
  ezama_loop();  //in ezama header file
  
  delay(100);
}
