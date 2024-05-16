#include <Ezama12.h>  // For ESP8266  ACS712-30A:66mv/A,ACS712-20A:100mv/A,ACS712-5A:185mv/A,
// This device reports current readings and can also interrupt power.

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "AC Monitor";
String ver = "9.0";

float voltage_array[] = {-99,0,0,0,0,0,0};
float current_array[] = {-99,0,0,0,0,0,0};
int input_pins[] = {-99, 36, 39, 34, 35, 32, 33};
int output_pins[] = {-99, 25, 22, 21, 19, 17, 16};
unsigned long previousMillis = 0;
const long interval = 20000; // interval in milliseconds

float sum_sqrd_current_array[] = {-99,0,0,0,0,0,0};
float max_current_array[] = {-99,0,0,0,0,0,0};
float rms_current_array[] = {-99,0,0,0,0,0,0};
float max_current_over_root_2_array[] = {-99,0,0,0,0,0,0};

//VIRTUAL:
//device_id/AConOffN: should accept: anything, "on", "off"
// onOff_array: onOff 0, 1: {NA, AC1, AC2, AC3, AC4, AC5, AC6};

int onOff_array[]  = {-99,0,0,0,0,0,0};  //Relay is NC!

//PHYSICAL:
//device_id/ACN: should accept: anything, "on", "off"


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
  state_json["vG"]        = "amp1,0,20;amp2,0,20;amp3,0,20;amp4,0,20;amp5,0,20;amp6,0,20";
  state_json["vL"]        = "1,6,AConOff";
  state_json["pL"]        = "1,6,AC";
  //state_json["pS"]        = "0,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj); 
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {

  //VIRTUAL MSGS: 
  //device_id/AConOffN: should accept: anything, "on", "off"

  for (int i = 1; i<=6; i++) {        
    if (topic == String(device_id) + "/" + "AConOff" + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 0;
      }
      if (msg == "off") {
        onOff_array[i] = 1;    
      }
    }
  }

  //PHYSICAL MESSAGES
  //device_id/ACN: should accept: anything, "on", "off"
  
  for (int i = 1; i<=6; i++) {        
    if (topic == String(device_id) + "/" + String("AC") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0g
      if (msg == "on") {
        onOff_array[i] = 0;
      } 
      if (msg == "off") {
        onOff_array[i] = 1;    
      }
    }
  }

  //Take action from non-sweeping commands, both virtual and physical.
  for (int i = 1; i<=6; i++) {
    digitalWrite(output_pins[i], onOff_array[i]);  // Turn off or on circuits
  }
}



// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}



//6 SETUP (pins)
void specific_connect() {
  String topic {};
  for (int i = 1; i<=6; i++) {        
    client.subscribe((String(device_id)+"/"+String("AConOff")+String(i)).c_str());
    }

  for (int i = 1; i<=6; i++) {        
    client.subscribe((String(device_id)+"/"+String("AC")+String(i)).c_str());
    }
}

void setup() { 
  ezama_setup();  //in ezama.h
  specific_connect();
  
  for (int i = 1; i<=6; i++) {
    pinMode(output_pins[i], OUTPUT);
    digitalWrite(output_pins[i], LOW);
  }
}

//7 MAIN LOOP

void take_readings() {
  for (int i = 1; i<=6; i++) {
    voltage_array[i] = analogRead(input_pins[i]) * 3.3 / 4096.0;
    current_array[i] = (voltage_array[i] -1.5) / 0.066;
    sum_sqrd_current_array[i] += current_array[i] * current_array[i];  //Doesn't seem to work.
    max_current_array[i] = max(max_current_array[i], current_array[i]);
    delay(2);  
  }
}


void loop() {
  ezama_loop();
  String topic {};

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 1; i<=6; i++) {
      sum_sqrd_current_array[i] = 0;
      max_current_array[i] = 0;
      rms_current_array[i] = 0;
      max_current_over_root_2_array[i] = 0; 
    }

    for (int i = 0; i < 500; i++) {
      take_readings();
    }
    
    for (int i = 1; i<=6; i++) {
      //rms_current_array[i] = max(sqrt(sum_sqrd_current_array[i]/500.0), max_current/sqrt(2));
      rms_current_array[i] = sqrt((1/500.0) * sum_sqrd_current_array[i]);
      max_current_over_root_2_array[i] = max_current_array[i] / sqrt(2.0);
    }

    for (int i = 1; i<=6; i++) {
      topic = String(device_id)+"/amp" + String(i);  
      client.publish(topic.c_str(), String(rms_current_array[i]).c_str());
      
      topic = String(device_id)+"/ampx" + String(i);  
      client.publish(topic.c_str(), String(max_current_over_root_2_array[i]).c_str());
    }
  }
}
