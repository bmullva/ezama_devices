#include <Ezama2.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "Relay";
String ver = "8.0";
const int d_pin [3] = {14, 12, 13};
const char* d_pin_name [3] = {"p14", "p12", "p13"};
int d_pin_reading [3] = {HIGH, HIGH, HIGH};
int d_pin_n1_reading [3] = {HIGH, HIGH, HIGH};
String d_pin_reading2state(int n) {
  if(n == 0) {return "on";}
  else {return "off";} 
  }
String timer [3] = {"off","off","off"};
unsigned long startMillis [3] = {0,0,0};
int clk [3] = {0,0,0};
String hld [3] = {"off","off","off"};
char light_1 [9] {};
char light_2 [9] {};
char light_3 [9] {};


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
  state_json["pin"]= "[p14,p12,p13]";
  state_json["action"] = "[click, dbl-click, hold, release]";
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.println(sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["cat"] = "controller";  // sensor, controller, actuator
  state_json["pin"]= "[p14,p12,p13]";
  state_json["cmds"] = "[click, dbl-click, hold, release]";
  state_json["ex"] = "on CONTROL topic, ['p14':'click']";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {

}


// 5 PUBLISH AND SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {
  
  if (pin_name == "p14" && String(light_1) != "control ") {
    Serial.print("Sending message to ");
    Serial.println(light_1);
    String topic = String(light_1);
    String pin_msg1 = "{\"p14\":\"" + pin_msg + "\"}";
    client.publish(topic.c_str(), pin_msg1.c_str());
  }

  if (pin_name == "p12" && String(light_2) != "control ") {
    Serial.print("Sending message to ");
    Serial.println(light_2);
    String topic = String(light_2);
    String pin_msg1 = "{\"p14\":\"" + pin_msg + "\"}";
    client.publish(topic.c_str(), pin_msg1.c_str());
  }

  if (pin_name == "p13" && String(light_3) != "control ") {
    Serial.print("Sending message to ");
    Serial.println(light_3);
    String topic = String(light_3);
    String pin_msg1 = "{\"p14\":\"" + pin_msg + "\"}";
    client.publish(topic.c_str(), pin_msg1.c_str());
  }

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


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  for (int i = 0; i < 8; i++) light_1[i] = char(EEPROM.read(light_1_id + i));
  for (int i = 0; i < 8; i++) light_2[i] = char(EEPROM.read(light_2_id + i));
  for (int i = 0; i < 8; i++) light_3[i] = char(EEPROM.read(light_3_id + i));

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  if (setting == 'n') {
  
  for(int i=0;i<sizeof(d_pin_reading)/sizeof(d_pin_reading[0]);i++) {d_pin_reading[i] = digitalRead(d_pin[i]);}  

  for (int i = 0; i<3; i++){
  
  if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == "off") {
    startMillis[i] = millis();
    timer[i] = "on";}
  
  if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == "on" && clk[i] == 0) {
    clk[i] += 1;
    publish_controls_json(String(d_pin_name[i]), "click");
    }

  else if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == "on" && clk[i] != 0) {
    clk[i] = 0;
    publish_controls_json(String(d_pin_name[i]), "dbl-click");
    timer[i] = "off";
    }    
  
  else if (millis() - startMillis[i] > 1000 && d_pin_reading[i] == LOW && timer[i] == "on" && hld[i] == "off") {
    hld[i] = "on";
    publish_controls_json(String(d_pin_name[i]), "hold");
    timer[i] = "off";
    }

  else if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && hld[i] == "on") {
    publish_controls_json(String(d_pin_name[i]), "release");
    hld[i] = "off";
    }
  
  if (millis() - startMillis[i] > 1000) {
    clk[i] = 0;
    timer[i] = "off";}
  
  d_pin_n1_reading[i] = d_pin_reading[i];
  }
  
  delay(50);
  }
}
