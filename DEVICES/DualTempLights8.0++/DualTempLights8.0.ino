#include <EzamaX.h> // This is for an ESP-01S, the light might be 34VDC


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "Dual-Temp Lights";  // This code is a variant of the light_box.
String ver = "8.0";

int click = 0;
int lux = 100;
int temp = 0;
int onOff = 0;
int dim = 0;
int brighten = 0;
int heat = 0;
int cool = 0;
const int pin1 = 1;
const int pin2 = 2;
int neg_temp (int temperature) {
  return 255 - temperature;
}

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
  state_json["onOff"] = onOff;
  state_json["lux"] = lux;
  state_json["temp"] = temp;
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
  state_json["action"]= "[onOff[1-0],lux%,temp%,hold,dim,brighten,heat,cool]";
  state_json["onOff"] = "1 or 0";
  state_json["lux"]= "bright 0-100";
  state_json["temp"] = "0-255, p2 is temp, p1 is neg_temp"; 
  state_json["ex"] = "on device ID topic, {'lux': 20}";
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

  if (jdoc.containsKey("click")) {click = jdoc["click"];}
  if (jdoc.containsKey("lux")) {lux = jdoc["lux"];}
  if (jdoc.containsKey("temp")) {temp = jdoc["temp"];}
  if (jdoc.containsKey("onOff")) {onOff = jdoc["onOff"];}
  if (jdoc.containsKey("dim")) {dim = jdoc["dim"];}
  if (jdoc.containsKey("brighten")) {brighten = jdoc["brighten"];}
  if (jdoc.containsKey("heat")) {heat = jdoc["heat"];}
  if (jdoc.containsKey("cool")) {cool = jdoc["cool"];}

  analogWrite(pin1, lux*neg_temp(temp)*onOff/100);
  analogWrite(pin2, lux*temp*onOff/100);  
}

void receive_onOff(String messageTemp){
    onOff = messageTemp.toInt();
    Serial.print("messageTemp: ");
    Serial.println(onOff);
    analogWrite(pin1, lux*neg_temp(temp)*onOff/100);
    analogWrite(pin2, lux*temp*onOff/100);
}

void receive_lux(String messageTemp){
    lux = 100 - messageTemp.toInt();
    Serial.print("messageTemp: ");
    Serial.println(lux);
    analogWrite(pin1, lux*neg_temp(temp)*onOff/100);
    analogWrite(pin2, lux*temp*onOff/100);
}

void receive_temp(String messageTemp){
    temp = messageTemp.toInt();
    Serial.print("messageTemp: ");
    Serial.println(temp);
    analogWrite(pin1, lux*neg_temp(temp)*onOff/100);
    analogWrite(pin2, lux*temp*onOff/100);
}

// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  //pinMode(3, OUTPUT);

  analogWrite(pin1, lux*neg_temp(temp)*onOff/100);
  analogWrite(pin2, lux*temp*onOff/100);  

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
   
if(click == 1){
  if (onOff == 0){onOff = 1;}
  else if (onOff == 1){onOff = 0;}
}
  
  
  if(brighten == 1){
    if (lux < 100) {
      lux = lux + 1;
    }
    analogWrite(1, lux*neg_temp(temp)*onOff/100);
    analogWrite(2, lux*temp*onOff/100);
  }

  if(dim == 1){
    if (lux > 0) {
      lux = lux - 1;
    }
    analogWrite(1, lux*neg_temp(temp)*onOff/100);
    analogWrite(2, lux*temp*onOff/100);
  }
  
  
  if(heat == 1){
    if (temp < 255) {
      temp = temp + 1;
    }
    analogWrite(1, lux*neg_temp(temp)*onOff/100);
    analogWrite(2, lux*temp*onOff/100);
  }

  if(cool == 1){
    if (temp > 0) {
      temp = temp - 1;
    }
    analogWrite(1, lux*neg_temp(temp)*onOff/100);
    analogWrite(2, lux*temp*onOff/100);
  }
  
  delay(20);
}
