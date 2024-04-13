#include <Ezama11.h>  // For ESP-32 DOIT ESP23
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Tri DB";
String ver = "11";

//const char* a_pin_name = "amps";
//float amp = (p36-1.65)/.1

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL: 1,2,3 Are Red, Green, and Blue
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 5-100
// onOff_array: onOff 0, 1: {NA, 1, 2, 3};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3};

int onOff_array[]  = {-99, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0};
 

//PHYSICAL: NONE!



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
  state_json["vL"]        = "1,3,onOff;1,3,lux";
  state_json["pL"]        = "1,3,";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  //device_id/onOffN: should accept: anything, "on", "off"
  // numbers go to device device_id/luxN, 5-100
  // device_id/tempN 0-255

  for (int i = 1; i<=3; i++) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      }
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
  }

  for (int i = 1; i<=3; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    

  //PHYSICAL MESSAGES: NONE

  analogWrite(0, dim_amt(lux_array[1]) * 255 * onOff_array[1] /100 );    //RED
  analogWrite(1,  dim_amt(lux_array[2]) * 255 * onOff_array[2] /100 );   //GREEN
  analogWrite(2, dim_amt(lux_array[3]) * 255 * onOff_array[3] /100 );    //BLUE
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  for (int i = 1; i <= 3; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }

  for (int i = 1; i<=3; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
}

void setup() { 
  Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  pinMode(0, OUTPUT);   //RED
  pinMode(1, OUTPUT);   //GREEN
  pinMode(2, OUTPUT);   //BLUE

  for (int i = 0; i<=2; i++) { 
    digitalWrite(i, HIGH);
  }

  delay(2000);
  
  for (int i = 0; i<=2; i++) { 
    digitalWrite(i, LOW);
  }

  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h
  
  delay(50);
}
