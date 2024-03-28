#include <Ezama8.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "30psi Depth Sensor";
String ver = "8.0";

const int a_pin = A0;
const char* a_pin_name = "ft depth";
int a_pin_reading = 0;  
int a_pin_n1_reading = 0;
float a_pin_reading2state(float n) {
  float v = n*3.3/1024.0;
  float psi = 7.5*v-3.75;
  float ft = 2.3*psi;
  float ft_rel = ft-.5;
  return ft_rel;
}
float depth;
float depth_array [20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int array_int = 0; 


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
  state_json["depth"] = depth;
  //state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = String(device_id) + "/depth";  
  client.publish(topic.c_str(), String(depth).c_str());
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;

  JsonArray vGageArray = state_json.createNestedArray("vGage");
  
  JsonArray firstArray = vGageArray.createNestedArray();
  firstArray.add("ft");
  firstArray.add(0);
  firstArray.add(70);

  //state_json["vLight"] = JsonArray{'start#', 'end#', 'name'};
  //state_json["pLight"] = JsonArray{'start#', 'end#', 'name'};
  //state_json["pPwtch"] = JsonArray{'start#', 'end#', 'name'};
  
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
  
  pinMode(A0, INPUT_PULLUP);

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  a_pin_reading  = analogRead(A0);
  float dpth = a_pin_reading2state(a_pin_reading);
  depth_array[array_int] = dpth;
  array_int += 1;
  float d = 0;
  int i;
  for (i=0; i<20; i++) {d += depth_array[i];}
  depth = d / 20;
  if (array_int == 20) {array_int = 0;}  
  delay(1000);
  
}
