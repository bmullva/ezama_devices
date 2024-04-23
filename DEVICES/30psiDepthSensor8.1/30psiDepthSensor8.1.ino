#include <Ezama11.h>  // For ESP-32 DOIT ESP23
//Warning!  This device returns up to a 5V analog signal.  Voltage divider probably wanted.

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "30psi Depth Sensor";
String ver = "8.1";

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
  state_json["type"]      = type_;
  state_json["ver"]       = ver;
  state_json["IP"]        = WiFi.localIP();
  state_json["vG"]        = "depth,0,20";
  //state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  //state_json["pL"]        = "1,12,;1,3,AC";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);

  topic = String(device_id)+"/depth";  
  client.publish(topic.c_str(), String(depth).c_str());

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
  
  topic = String(device_id)+"/"+String("depth");
  client.subscribe(topic.c_str());
  
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  
  pinMode(A0, INPUT_PULLUP);
 
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

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
