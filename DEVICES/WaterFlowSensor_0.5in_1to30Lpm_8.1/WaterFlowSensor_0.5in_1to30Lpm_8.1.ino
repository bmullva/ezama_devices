#include <Ezama11.h>  // For ESP-32 DOIT ESP23
#include <Filters.h>
//Flow sensor is a 5V device, and returns a 

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Water Flow 0.5in - 30Lpm";
String ver = "8.1";

float lpm {};
float gpm {};
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
  state_json["type"]      = type_;
  state_json["ver"]       = ver;
  state_json["IP"]        = WiFi.localIP();
  state_json["vG"]        = "gpm,0,30";
  //state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  //state_json["pL"]        = "1,12,;1,3,AC";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  
  topic = String(device_id)+"/gpm";  
  client.publish(topic.c_str(), String(gpm).c_str());


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
  
  topic = String(device_id)+"/"+String("gpm");
  client.subscribe(topic.c_str());

}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  
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
  lpm = pulses * (1/7.5) * (1/60.0);
  gpm = lpm * 0.264172;
  
  delay(1000);
}
