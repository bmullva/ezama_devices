#include <Ezama11.h>  // For ESP-32 DOIT ESP23
//#include <Filters.h>
//DO LIGHT FIRST, THEN TOGGLE

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "AC Switchbox";
String ver = "10.1";

int d_pin_reading [1]         = {HIGH};
int d_pin_n1_reading [1]      = {HIGH};
//unsigned long startMillis [1] = {0};
//int clk [1]                   = {0};
//int rel [1]                   = {0};


//VIRTUAL:
//lux 1-100
//onOff 0, 1
//temp 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, AC1, AC2, AC3};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3, lux4, lux5, lux6, lux7, lux8, lux9, lux10, lux11, lux12};
// temp_array:  {temp11, temp12};

int onOff_array[]  = {-99, 0};
//int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//int temp_array[]   = {0, 0};
 

//PHYSICAL:
//device_id/N N=1-12: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=13,14 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
// +1 -1 or 0: lt_array:  NA, lm1, lm2, lm3, lm4, lm5, lm6, lm7, lm8, lm9, lm10, lm11Lux, lm12Lux, lm11Temp, lm12Temp
//int lt_array []    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//int mom_pin_array[]= {-99,13,4,14,16,27,17,26,18,25,19};



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
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.println(sj);
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  // anything goes to device_id/NonOff, 1 ("click")
  // numbers go to device device_id/Nlux, 5-100
  // device_id/Ntemp 0-255

  for (int i = 1; i<=1; i++) {        
    if (topic == String(device_id) + "/" + "AConOff" + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
    }
  }
  

  //PHYSICAL MESSAGES
  //these are for the "NAC" topics from a physical switch.
  for (int i = 1; i<=1; i++) {        
    if (topic == String(device_id) + "/" + String("AC") + String(i)) {
        onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0g
    }
  }

  digitalWrite(1, onOff_array[1]);   // AC1
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
  Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  pinMode(0, INPUT_PULLUP);
  pinMode(1, OUTPUT);  //AC1
  digitalWrite(1, HIGH);    //AC1
  delay(5000);
  digitalWrite(1, LOW);    //AC1
  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h

  // Loop through an array of digital pins, read their states, and store the readings
  //for(int i=0;i<sizeof(d_pin_reading)/sizeof(d_pin_reading[0]);i++) {
    int i = 0;
    d_pin_reading[i] = digitalRead(i);

    // Loop through a set of conditions for the pins
    if (d_pin_n1_reading[i] != d_pin_reading[i]) {
          publish_controls_json(String(i), "click");
          onOff_array[i+1] = 1-onOff_array[i+1];
          d_pin_n1_reading[i] = d_pin_reading[i];
          digitalWrite(i+1, onOff_array[1]);   // AC1
    }
  //}
  
  delay(50);
}
