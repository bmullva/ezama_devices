#include <Ezama11.h>  // For ESP-01S
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Dual DB";
String ver = "11";

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 1-100
//device_id/temp: should accept: 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3, lux4};
// temp_array:  {temp1, temp2, temp3, temp4};

int onOff_array[]  = {-99, 0, 0};
int lux_array[]    = {-99, 0, 0};
int temp_array[]   = {-99, 0, 0};
 

//PHYSICAL:
//device_id/N N=1-4 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
// +1 -1 or 0: lt_array:  NA, lm1, lm2, lm1Temp, lm2Temp
int lt_array []    = {-99, 0, 0, 0, 0};



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
  state_json["vL"]        = "1,2,onOff;1,2,lux;1,2,temp";
  state_json["pL"]        = "1,2,";
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
  //device_id/luxN: should accept: 5-100
  //device_id/tempN: should accept: 0-255

  for (int i = 1; i<=2; i++) {        
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

  for (int i = 1; i<=2; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 5-100
    }
  }
    
  for (int i = 1; i<=2; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-2 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //lm1Lux, lm2Lux, lm1Temp, lm2Temp

  for (int i = 1; i<=2; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {
        // msg to these topics should be "on", "off", "dim", "brighten", "heat", "cool", "release"
               
        if (msg == "on") {
          onOff_array[i] = 1;
        }

        if (msg == "off") {
          onOff_array[i] = 0;
        }
        
        if (msg == "dim") {
          lt_array[i] = 1;
        }

        if (msg == "brighten") {
          lt_array[i] = -1;
        }

         if (msg == "heat") {
          lt_array[i+2] = 1;
        }
        
        if (msg == "cool") {
          lt_array[i+2] = -1;
        }

        if (msg == "release") {
          lt_array[i] = 0;
          lt_array[i+2] = 0;
        }
    }
  }


  //NAC DOES NOT EXIST on this Light Hub

  analogWrite(0, dim_amt(lux_array[1]) * (255-temp_array[1]) * onOff_array[1] /100 );    //1Low
  analogWrite(1, dim_amt(lux_array[1]) *     temp_array[1]   * onOff_array[1] /100 );    //1High
  analogWrite(2, dim_amt(lux_array[2]) * (255-temp_array[2]) * onOff_array[2] /100 );    //2Low
  analogWrite(3, dim_amt(lux_array[2]) *     temp_array[2]   * onOff_array[2] /100 );    //2High

}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  for (int i = 1; i<=2; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
    }
  
  for (int i = 1; i <= 2; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }

  for (int i = 1; i<=2; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 1; i<=2; i++) {        
    client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
    }
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  //pinMode(A0, INPUT);
  pinMode(0, OUTPUT);  // 1LOW
  pinMode(1, OUTPUT);  // 1HIGH
  pinMode(2, OUTPUT);  // 2LOW
  pinMode(3, OUTPUT);  // 2HIGH

  digitalWrite(0, HIGH);    // 1LOW
  digitalWrite(1, HIGH);    // 1HIGH
  digitalWrite(2, HIGH);    // 2LOW
  digitalWrite(3, HIGH);    // 2HIGH

  delay(2000);

  digitalWrite(0, LOW);    // 1LOW
  digitalWrite(1, LOW);    // 1HIGH
  digitalWrite(2, LOW);    // 2LOW
  digitalWrite(3, LOW);    // 2HIGH

  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm1Lux, lm2Lux, lm1Temp, lm2Temp
void loop() {
  ezama_loop();  //in ezama.h
  
  for(int i=1;i<=2;i++) {
    if(lt_array[i] == 1){   // like increasing the dim slider
      if (lux_array[i] >= 0 && lux_array[i] < 94) {
        lux_array[i] += 1;
        if (i == 1) {
          analogWrite(0, dim_amt(lux_array[1]) * (255-temp_array[1])    * onOff_array[1] /100 );    //1Low
          analogWrite(1, dim_amt(lux_array[1]) *    temp_array[1]       * onOff_array[1] /100 );    //1High
        }
        if (i == 2){
          analogWrite(2, dim_amt(lux_array[2]) * (255-temp_array[2])   * onOff_array[2] /100 );     //2Low
          analogWrite(3, dim_amt(lux_array[2]) *    temp_array[2]      * onOff_array[2] /100 );     //2High
        }
      }
    }
    if(lt_array[i] == -1){  // like decreasing the dim slider
      if (lux_array[i] > 5 && lux_array[i] <= 100) {
        lux_array[i] -= 1;
        if (i == 1) {
          analogWrite(0,  dim_amt(lux_array[1]) *     temp_array[1]   * onOff_array[1] /100 );    //1Low
          analogWrite(1, dim_amt(lux_array[1]) * (255-temp_array[1]) * onOff_array[1] /100 );     //1High
        }
        if (i == 2){
          analogWrite(2, dim_amt(lux_array[2]) *     temp_array[2]   * onOff_array[2] /100 );     //2Low
          analogWrite(3, dim_amt(lux_array[2]) * (255-temp_array[2]) * onOff_array[2] /100 );     //2High
        }
      }
    }
    if(lt_array[i+2] == 1){   // like increasing the temp slider  
      if (temp_array[i] >= 0 && temp_array[i] < 255) {
        temp_array[i] += 5;
        if (temp_array[i] >= 255) {temp_array[i] = 255;}
        if (i == 1) {
          analogWrite(0,  dim_amt(lux_array[1]) *     temp_array[1]   * onOff_array[1] /100 );    //1Low
          analogWrite(1, dim_amt(lux_array[1]) * (255-temp_array[1]) * onOff_array[1] /100 );     //1High
        }
        if (i == 2){
          analogWrite(2, dim_amt(lux_array[2]) *     temp_array[2]   * onOff_array[2] /100 );     //2Low
          analogWrite(3, dim_amt(lux_array[2]) * (255-temp_array[2]) * onOff_array[2] /100 );     //2High
        }
      }
    }
    if(lt_array[i+2] == -1){  // like decreasing the temp slider 
      if (temp_array[i] > 0 && temp_array[i] <= 255) {
        temp_array[i] -= 5;
        if (temp_array[i] <= 0) {temp_array[i] = 0;}
        if (i == 1) {
          analogWrite(0,  dim_amt(lux_array[1]) *     temp_array[1]   * onOff_array[1] /100 );    //1Low
          analogWrite(1, dim_amt(lux_array[1]) * (255-temp_array[1]) * onOff_array[1] /100 );     //1High
        }
        if (i == 2){
          analogWrite(2, dim_amt(lux_array[2]) *     temp_array[2]   * onOff_array[2] /100 );     //2Low
          analogWrite(3, dim_amt(lux_array[2]) * (255-temp_array[2]) * onOff_array[2] /100 );     //2high
        }
      }
    }
  }

  delay(50);
}
