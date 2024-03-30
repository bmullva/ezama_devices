#include <Ezama10_1.h>  // For ESP-32 DOIT ESP23
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Light Hub";
String ver = "10.0";

//const char* a_pin_name = "amps";
//float amp = (p36-1.65)/.1

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//lux 1-100
//onOff 0, 1
//temp 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, AC1, AC2, AC3};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3, lux4, lux5, lux6, lux7, lux8, lux9, lux10, lux11, lux12};
// temp_array:  {temp11, temp12};

int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temp_array[]   = {0, 0};
 

//PHYSICAL:
//device_id/N N=1-12: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=13,14 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
// +1 -1 or 0: lt_array:  NA, lm1, lm2, lm3, lm4, lm5, lm6, lm7, lm8, lm9, lm10, lm11Lux, lm12Lux, lm11Temp, lm12Temp
int lt_array []    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int mom_pin_array[]= {-99,13,4,14,16,27,17,26,18,25,19};



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
  state_json["vG"]        = "amp,0,20";
  state_json["vL"]        = "1,12,onOff;1,12,lux;11,12,temp;1,3,AConOff";
  state_json["pL"]        = "1,12,;1,3,AC";
  //state_json["pS"]        = "1,4,onOff";
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

  for (int i = 1; i<=12; i++) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
    }
  }

  for (int i = 1; i<=3; i++) {        
    if (topic == String(device_id) + "/" + "AConOff" + String(i)) {
      onOff_array[i+12] = 1-onOff_array[i+12];  // any msg will switch between 1 and 0
    }
  }
  
  for (int i = 1; i<=12; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 11; i<=12; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i-11] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-10 should accept "hold", "click", "click-hold", "dbl-click", "release"
  //change to also accept "on", "off", "dim", "brighten"
  
  for (int i = 1; i<=10; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {

        if (msg == "hold") {
          lt_array[i] = 1;
          Serial.println("Received dim signal in controls.");
        }
        
        if (msg == "click") {
          onOff_array[i] = 1 - onOff_array[i];
          if (onOff_array[i] == 0) {lux_array[i] = 0;} // resets to max lum after turning off
        }

        if (msg == "click-hold") {
          lt_array[i] = -1;
          Serial.println("Received brighten signal in controls");
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
          Serial.println("Received release signal in controls");
        }        
    }
  }

  //device_id/N N=11,12 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //lm11Lux, lm12Lux, lm11Temp, lm12Temp

  for (int i = 11; i<=12; i++) {        
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

  //these are for the "NAC" topics from a physical switch.
  for (int i = 1; i<=3; i++) {        
    if (topic == String(device_id) + "/" + String("AC") + String(i)) {
        onOff_array[i+12] = 1-onOff_array[i+12];  // any msg will switch between 1 and 0g
    }
  }


  analogWrite(13, dim_amt(lux_array[1]) * 255 * onOff_array[1] /100 );    //01
  analogWrite(4,  dim_amt(lux_array[2]) * 255 * onOff_array[2] /100 );    //02
  analogWrite(14, dim_amt(lux_array[3]) * 255 * onOff_array[3] /100 );    //03
  analogWrite(16, dim_amt(lux_array[4]) * 255 * onOff_array[4] /100 );    //04
  analogWrite(27, dim_amt(lux_array[5]) * 255 * onOff_array[5] /100 );    //05
  analogWrite(17, dim_amt(lux_array[6]) * 255 * onOff_array[6] /100 );    //06
  analogWrite(26, dim_amt(lux_array[7]) * 255 * onOff_array[7] /100 );    //07
  analogWrite(18, dim_amt(lux_array[8]) * 255 * onOff_array[8] /100 );    //08
  analogWrite(25, dim_amt(lux_array[9]) * 255 * onOff_array[9] /100 );    //09
  analogWrite(19, dim_amt(lux_array[10]) * 255 * onOff_array[10] /100 );  //10

  analogWrite(32,  dim_amt(lux_array[11]) *     temp_array[0]   * onOff_array[11] /100 );    //11High
  analogWrite(33, dim_amt(lux_array[11]) * (255-temp_array[0]) * onOff_array[11] /100 );     //11Low
  analogWrite(23, dim_amt(lux_array[12]) *     temp_array[1]   * onOff_array[12] /100 );     //12High
  analogWrite(21, dim_amt(lux_array[12]) * (255-temp_array[1]) * onOff_array[12] /100 );     //12Low

  digitalWrite(22, onOff_array[13]);   // AC1
  digitalWrite(2,  onOff_array[14]);   // AC2
  digitalWrite(12, onOff_array[15]);   // AC3
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  //if (!client.connected()) {reconnect();}
  
  topic = String(device_id)+"/"+String("amp");
  client.subscribe(topic.c_str());
  if (!client.subscribe(topic.c_str())) {
        Serial.println("Failed to subscribe to topic: " + topic);
  }

  for (int i = 1; i<=12; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
    }
  
  //for (int i = 1; i<=12; i++) {        
  //  client.subscribe((String(device_id)+"/"+String(i)+String("onOff")).c_str());
  //  }

  for (int i = 1; i <= 12; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
    if (!client.subscribe(topic.c_str())) {
        Serial.println("Failed to subscribe to topic: " + topic);
    }
  }

  for (int i = 1; i<=12; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 11; i<=12; i++) {        
    client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
    }
    
  for (int i = 1; i<=3; i++) {        
    client.subscribe((String(device_id)+"/"+String("AConOff")+String(i)).c_str());
    }

  for (int i = 1; i<=3; i++) {        
    client.subscribe((String(device_id)+"/"+String("AC")+String(i)).c_str());
    }
}

void setup() { 
  Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  //pinMode(A0, INPUT);
  pinMode(32, OUTPUT);  //LED11HIGH
  pinMode(33, INPUT);   //LED11LOW
  pinMode(25, OUTPUT);  //LED09
  pinMode(26, OUTPUT);  //LED07
  pinMode(27, OUTPUT);  //LED05
  pinMode(14, OUTPUT);  //LED3
  pinMode(12, OUTPUT);  //AC3
  pinMode(13, OUTPUT);  //LED01
  pinMode(23, OUTPUT);  //LED12HIGH
  pinMode(22, OUTPUT);  //AC1
  pinMode(21, OUTPUT);  //LED12LOW
  pinMode(19, OUTPUT);  //LED10
  pinMode(18, OUTPUT);  //LED08
  pinMode(17, OUTPUT);  //LED06
  pinMode(16, OUTPUT);  //LED04
  pinMode(4, OUTPUT);   //LED02
  pinMode(2, OUTPUT);   //AC2

  for (int i = 1; i<=10; i++) { 
    digitalWrite(mom_pin_array[i], HIGH);    //01-10
  }
  digitalWrite(32, HIGH);    //11High
  digitalWrite(33, HIGH);    //11Low
  digitalWrite(23, HIGH);    //12High
  digitalWrite(21, HIGH);    //12Low
  digitalWrite(22, HIGH);    //AC1
  digitalWrite(2,  HIGH);    //AC2
  digitalWrite(12, HIGH);    //AC3
  delay(5000);
  for (int i = 1; i<=10; i++) { 
    digitalWrite(mom_pin_array[i], LOW);    //01-10
  }
  digitalWrite(32, LOW);    //11High
  digitalWrite(33, LOW);    //11Low
  digitalWrite(23, LOW);    //12High
  digitalWrite(21, LOW);    //12Low
  digitalWrite(22, LOW);    //AC1
  digitalWrite(2,  LOW);    //AC2
  digitalWrite(12, LOW);    //AC3

  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h
  
  for(int i=1;i<=10;i++) {
    if(lt_array[i] == 1){   // like increasing the dim slider
      Serial.print("lt_array[i]=1 and lux[i] is:");
      Serial.println(lux_array[i]);
      if (lux_array[i] >= 0 && lux_array[i] < 94) {
        lux_array[i] += 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
      }      
    }
    if(lt_array[i] == -1){  // like decreasing the dim slider
      Serial.print("lt_array[i]=1 and i is:");
      Serial.println(i);
      if (lux_array[i] > 0 && lux_array[i] <= 100) {
        lux_array[i] -= 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
      }
    }
  }
  
  
  for(int i=11;i<=12;i++) {
    if(lt_array[i] == 1){   // like increasing the dim slider
      Serial.print("lt_array[i]=1 and i is:");
      Serial.println(i);
      if (lux_array[i] >= 0 && lux_array[i] < 94) {
        lux_array[i] += 1;
        if (i == 11) {
          analogWrite(32,  dim_amt(lux_array[11]) *     temp_array[0]   * onOff_array[11] /100 );    //11High
          analogWrite(33, dim_amt(lux_array[11]) * (255-temp_array[0]) * onOff_array[11] /100 );     //11Low
        }
        if (i == 12){
          analogWrite(23, dim_amt(lux_array[12]) *     temp_array[1]   * onOff_array[12] /100 );     //12High
          analogWrite(21, dim_amt(lux_array[12]) * (255-temp_array[1]) * onOff_array[12] /100 );     //12Low
        }   
        Serial.println("I am dimming in the 11-12 main loop");
      }
    }
    if(lt_array[i] == -1){  // like decreasing the dim slider
      Serial.print("lt_array [i]=-1 and i is:");
      Serial.println(i);
      if (lux_array[i] > 5 && lux_array[i] <= 100) {
        lux_array[i] -= 1;
        if (i == 11) {
          analogWrite(32,  dim_amt(lux_array[11]) *     temp_array[0]   * onOff_array[11] /100 );    //11High
          analogWrite(33, dim_amt(lux_array[11]) * (255-temp_array[0]) * onOff_array[11] /100 );     //11Low
        }
        if (i == 12){
          analogWrite(23, dim_amt(lux_array[12]) *     temp_array[1]   * onOff_array[12] /100 );     //12High
          analogWrite(21, dim_amt(lux_array[12]) * (255-temp_array[1]) * onOff_array[12] /100 );     //12Low
        }   
        Serial.println("I am brightening in the 11-12 main loop");
      }
    }
    if(lt_array[i+2] == 1){   // like increasing the temp slider
      Serial.print("lt_array[i]=1 and i is:");
      Serial.println(i);     
      if (temp_array[i-11] >= 0 && temp_array[i-11] < 255) {
        temp_array[i-11] += 5;
        if (temp_array[i-11] >= 255) {temp_array[i-11] = 255;}
        if (i == 11) {
          analogWrite(32, dim_amt(lux_array[11]) *     temp_array[0]   * onOff_array[11] /100 );     //11High
          analogWrite(33, dim_amt(lux_array[11]) * (255-temp_array[0]) * onOff_array[11] /100 );     //11Low
        }
        if (i == 12){
          analogWrite(23, dim_amt(lux_array[12]) *     temp_array[1]   * onOff_array[12] /100 );     //12High
          analogWrite(21, dim_amt(lux_array[12]) * (255-temp_array[1]) * onOff_array[12] /100 );     //12Low
        }    
        Serial.println("I am heating in the 11-12 main loop");
      }
    }
    if(lt_array[i+2] == -1){  // like decreasing the temp slider
      Serial.print("lt_array [i]=-1 and i is:");
      Serial.println(i);
      
      if (temp_array[i-11] > 0 && temp_array[i-11] <= 255) {
        temp_array[i-11] -= 5;
        if (temp_array[i-11] <= 0) {temp_array[i-11] = 0;}
        if (i == 11) {
          analogWrite(32, dim_amt(lux_array[11]) *     temp_array[0]   * onOff_array[11] /100 );     //11High
          analogWrite(33, dim_amt(lux_array[11]) * (255-temp_array[0]) * onOff_array[11] /100 );     //11Low
        }
        if (i == 12){
          analogWrite(23, dim_amt(lux_array[12]) *     temp_array[1]   * onOff_array[12] /100 );     //12High
          analogWrite(21, dim_amt(lux_array[12]) * (255-temp_array[1]) * onOff_array[12] /100 );     //12Low
        } 
        
        Serial.println("I am cooling in the 11-12 main loop");
      }
    }
  }

  delay(50);
}
