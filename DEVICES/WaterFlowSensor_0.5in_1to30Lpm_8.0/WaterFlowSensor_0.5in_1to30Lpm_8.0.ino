#include <Ezama2.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "Water Flow Sensor(0.5in_1to30Lpm)";
String ver = "8.0";

float liters {};
volatile int pulses = 0;    // count pulses
void ICACHE_RAM_ATTR isr() { // ISR to increment count variable when pin goes high
  pulses++;
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
  state_json["liters"] = liters;
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = device_id+"/gpm";  
  client.publish(topic.c_str(), gpm);
  
}

// sensor, controller, actuator, actuator-sensor
// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  
  state_json["vGage"] = "[[\"gpm\", 0, 100]]";
  //state_json["vLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pPwtch"] = "[['start#', 'end#', 'name']]";
  
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {
  
     if(messageTemp == "on") {
    digitalWrite(13, HIGH);
    Serial.print("Writing 13 to HIGH");
   }

   if(messageTemp == "off") {
    digitalWrite(13, LOW);
    Serial.print("Writing 13 to LOW");
   }   
  
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(14), isr, RISING); // Attach ISR to pin's rising edge interrupt

}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  // plastic sensor: use following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  liters = pulses;
  liters /= 7.5;
  liters /= 60.0;
  
  delay(1000);
  
}
