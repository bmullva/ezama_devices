#include <Ezama12.h>  // For D1Mini
//DO LIGHT FIRST, THEN TOGGLE

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "AC Switchbox D1Mini";
String ver = "10.4";

int d_pin_reading [1]         = {HIGH};
int d_pin_n1_reading [1]      = {HIGH};

//VIRTUAL:
//device_id/AConOffN: should accept: anything, "on", "off"
// onOff_array: onOff 0, 1: {NA, AC1};

int onOff_array[]  = {-99, 0};
 

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
  //state_json["vG"]        = "amp,0,20";
  state_json["vL"]        = "1,1,AConOff";
  state_json["pL"]        = "1,1,AC";
  state_json["pS"]        = "1,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  // device_id/AConOffN: should accept: anything, "on", "off"

  for (int i = 1; i<=1; i++) {        
    if (topic == String(device_id) + "/" + "AConOff" + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
  }
  

  //PHYSICAL MESSAGES
  // // device_id/ACN: should accept: anything, "on", "off"
  for (int i = 1; i<=1; i++) {        
    if (topic == String(device_id) + "/" + String("AC") + String(i)) {
        onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0g
        if (msg == "on") {
          onOff_array[i] = 1;
        }
        
        if (msg == "off") {
          onOff_array[i] = 0;    
        }
    }
  }

  digitalWrite(14, onOff_array[1]);   // AC1
}


// 5 PUBLISH AND SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {
  String topic = String(device_id) + "/1";
  client.publish(topic.c_str(), pin_msg.c_str());
}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  for (int i = 1; i<=1; i++) {        
    client.subscribe((String(device_id)+"/"+String("AConOff")+String(i)).c_str());
    }

  for (int i = 1; i<=1; i++) {        
    client.subscribe((String(device_id)+"/"+String("AC")+String(i)).c_str());
    }
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  pinMode(4, INPUT_PULLUP);
  pinMode(14, OUTPUT);  //AC1
  digitalWrite(14, HIGH);    //AC1
  delay(5000);
  digitalWrite(14, LOW);    //AC1
  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h

  // Loop through an array of digital pins, read their states, and store the readings
  //for(int i=0;i<sizeof(d_pin_reading)/sizeof(d_pin_reading[0]);i++) {
    int i = 0;
    d_pin_reading[i] = digitalRead(4);

    // Loop through a set of conditions for the pins
    if (d_pin_n1_reading[i] != d_pin_reading[i]) {
          publish_controls_json(String(i), "click");
          onOff_array[i+1] = 1-onOff_array[i+1];
          d_pin_n1_reading[i] = d_pin_reading[i];
          digitalWrite(14, onOff_array[1]);   // AC1
    }
  //}
  
  delay(50);
}
