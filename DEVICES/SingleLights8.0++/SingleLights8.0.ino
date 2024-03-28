#include <Ezama2.h> // This is for an ESP-01S


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "Single Lights";
String ver = "8.0";

String dimmer1 = "off";
String dimmer2 = "off";
String dimmer3 = "off";
int p1 = 0;
int p2 = 0;
int p3 = 0;
int p_array[] = {p1, p2, p3};
int n_array[] = {1, 2, 3};
String d_array[] = {dimmer1, dimmer2, dimmer3};


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
  state_json["p1"] = p_array[0];
  state_json["p2"] = p_array[1];
  state_json["p3"] = p_array[2];
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["cat"] = "actuator";  // sensor, controller, actuator
  state_json["pins"] = "[p1, p2, p3]";
  state_json["actions"]= "[switch, on, off, dim, stop, brighten, 0-255]";
  state_json["ex"] = "on device ID topic, ['p1':'dim']";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
  
  StaticJsonDocument<256> jdoc;    
  DeserializationError error = deserializeJson(jdoc, messageTemp);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String p1_msg;
  if (jdoc.containsKey("p14")) {
    p1_msg = jdoc["p14"].as<String>();
  } else if (jdoc.containsKey("p1")) {
    p1_msg = jdoc["p1"].as<String>();
  }
  String p2_msg = jdoc["p2"];
  String p3_msg = jdoc["p3"];
  String msg_array[] = {p1_msg, p2_msg, p3_msg};
      
  for (int i = 0; i<3; i++) {        
    if (msg_array[i] == "click" && p_array[i] != 0) {
      analogWrite(n_array[i], 0);
      p_array[i] = 0;
    }
    else if (msg_array[i] == "click" && p_array[i] == 0){
      analogWrite(n_array[i], 255); //CAN GO UP TO 255
      p_array[i] = 255;
    }      
    if(msg_array[i] == "on"){
      analogWrite(n_array[i], HIGH);
      p_array[i] = HIGH;
    }
    if(msg_array[i] == "off"){
      analogWrite(n_array[i], LOW);
      p_array[i] = LOW;
    }
    if(msg_array[i] == "dim"){
      d_array[i] = "dim";
    }
    if(msg_array[i] == "release"){
      d_array[i] = "release";
    }
    if(msg_array[i] == "brighten"){
      d_array[i] = "brighten";
    }
    if(isDigit(msg_array[i][0])){
      p_array[i] = msg_array[i].toInt();
      analogWrite(n_array[i], p_array[i]);
    }
  }
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  for(int i=0;i<3;i++) {
    if(d_array[i] == "dim"){
      if (p_array[i] == 0) {p_array[i] = 255+5;}
      p_array[i] -= 5;
      if (p_array[i] < 5) {
        d_array[i] = "release";
        p_array[i] = 0;    
      }
      analogWrite(n_array[i], p_array[i]);
    }
    if(d_array[i] == "brighten"){
      p_array[i] += 5;
      if (p_array[i] > 255) {
        d_array[i] = "release";
        p_array[i] = 255;
      }
      analogWrite(n_array[i], p_array[i]);
    }
  }
}
