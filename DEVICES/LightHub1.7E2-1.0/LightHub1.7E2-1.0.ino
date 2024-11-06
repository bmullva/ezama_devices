#include <Ezama12.h>  // For ESP-32 DOIT ESP23
//#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Light Hub 1.7E2";
String ver = "1.0";

float amp {};
float voltage {};
// The code needs to be updated to 1.5V for 3V power.

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/AConOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 0-95
//device_id/tempN: should accept: 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3, lux4, lux5, lux6, lux7, lux8, lux9, lux10};
// temp_array:  {temp9, temp10};
int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temp_array[]   = {-99,-9,-9,-9,-9,-9,-9,-9,-9, 0, 0};


int mom_pin_array[]= {-99,13,4,14,16,27,17,26,18};
int low_pins[]  = {-99,-9,-9,-9,-9,-9,-9,-9,-9, 25, 19};
int high_pins[] = {-99,-9,-9,-9,-9,-9,-9,-9,-9, 33, 23};
void t_write(int i) {
  analogWrite(low_pins[i],  dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  analogWrite(high_pins[i], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High  
}


//PHYSICAL:
//device_id/N N=1-8: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=9,10 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
//device_id/ACN: should accept: anything, "on", "off"
// +1 -1 or 0: lt_array: NA, lm1,lm2,lm3,lm4,lm5,lm6,lm7,lm8,lux9,lux10,temp9,temp10 
int lt_array [] = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



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
  state_json["vL"]        = "1,10,onOff;1,10,lux;9,10,temp";
  state_json["pL"]        = "1,10,";
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
  //device_id/luxN: should accept: 0-95
  //device_id/tempN: should accept: 0-255

  for (int i = 1; i<=10; i++) {        
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
  
  for (int i = 1; i<=10; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 9; i<=10; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-8 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  
  for (int i = 1; i<=8; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {

        if (msg == "hold") {
          lt_array[i] = 1;
        }
        
        if (msg == "click") {
          onOff_array[i] = 1 - onOff_array[i];
        }

        if (msg == "click-hold") {
          lt_array[i] = -1;
        }
        
        if (msg == "dbl-click") {
          
        }

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
        
        if (msg == "release") {
          lt_array[i] = 0;
        }        
    }
  }

  //device_id/N N=9-10 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //NA, lux9, lux10, temp9, temp10

  for (int i = 9; i<=10; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {
        // msg to these topics should be "click", "on", "off", "dim", "brighten", "heat", "cool", "release"
               
        if (msg == "click") {
          onOff_array[i] = 1 - onOff_array[i];
        }
        
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

  //Take action from non-"sweeping" commands, both virtual and physical.
  //"Sweeping" commands are handled in the main loop.
  for (int i = 1; i<=8; i++) {
    analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }

  for (int i = 9; i<=10; i++) {
    t_write(i);
  }
}


// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  for (int i = 1; i<=10; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
    }

  for (int i = 1; i <= 10; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }

  for (int i = 1; i<=10; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 9; i<=10; i++) {        
    client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
    }
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
 
  for (int i = 1; i<=8; i++) { 
    digitalWrite(mom_pin_array[i], OUTPUT);    //01-8
    digitalWrite(mom_pin_array[i], HIGH);      //01-8
  }
  
  for (int i = 1; i<=2; i++) { 
    digitalWrite(low_pins[i], OUTPUT);  //Low temp, High temp
    digitalWrite(low_pins[i], HIGH);
    digitalWrite(high_pins[i], OUTPUT);  //Low temp, High temp
    digitalWrite(high_pins[i], HIGH);
  } 
  
  delay(2000);
  
  for (int i = 1; i<=8; i++) { 
    digitalWrite(mom_pin_array[i], LOW);    //01-8
  }

  for (int i = 1; i<=2; i++) { 
    digitalWrite(low_pins[i], LOW);    //Low temp, High temp, and AC pins
    digitalWrite(high_pins[i], LOW);    //Low temp, High temp, and AC pins
  }
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h

  for(int i=1;i<=8;i++) {
    if(lt_array[i] == 1 && lux_array[i] < 99){   // like increasing the dim slider
        lux_array[i] += 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );    
    }
    if(lt_array[i] == -1 && lux_array[i] > 0){  // like decreasing the dim slider
        lux_array[i] -= 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
    }
  }
  
  for(int i=9;i<=10;i++) {  
    if (lt_array[i] == 1 && lux_array[i] < 95) {  // like increasing the dim slider
       lux_array[i] += 1;
       t_write(i);
    }    
    if(lt_array[i] == -1 && lux_array[i] > 0)  {  // like decreasing the dim slider
        lux_array[i] -= 1;
        t_write(i);
    }
    if(lt_array[i+2] == 1 && temp_array[i] < 255)  {   // like increasing the temp slider
        temp_array[i] += 5;
        if (temp_array[i] >= 255) {temp_array[i] = 255;}
        t_write(i);
    }
    if(lt_array[i+2] == -1 && temp_array[i] > 0){  // like decreasing the temp slider
        temp_array[i] -= 5;
        if (temp_array[i] <= 0) {temp_array[i] = 0;}
        t_write(i);
    }
  }        

  delay(50);
}
