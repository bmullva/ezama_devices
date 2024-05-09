#include <Ezama11.h>  // For ESP-32 DOIT ESP23
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Light Hub";
String ver = "11.0";

float voltage_array[] = {-99,0,0,0,0,0};
float current_array[] = {-99,0,0,0,0,0};
int input_pins[] = {36, 39, 34, 35, 15, 12};
unsigned long previousMillis = 0;
const long interval = 20000; // interval in milliseconds
float sum_sqrd_current_array[] = {0,0,0,0,0,0};
//float max_current_array[] = {0,0,0,0,0,0};
float rms_current_array[] = {0,0,0,0,0,0};
//float max_current_over_root_2_array[] = {0,0,0,0,0,0};

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/AConOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 5-100
//device_id/tempN: should accept: 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
// lux_array:  lux 0-100 {NA, lux1, lux2, lux3, lux4, lux5, lux6, lux7, lux8, lux9, lux10, lux11, lux12};
// temp_array:  {temp11, temp12};
int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temp_array[]   = {-99, 0, 0};

int mom_pin_array[]= {-99, 13, 2, 14, 4, 27, 16, 26, 17, 25, 5};
int low_pins[]  = {-99, 33, 18};
int high_pins[] = {-99, 32, 19};
void t_write(int i) {
  analogWrite(low_pins[i-10],  dim_amt(lux_array[i]) * (255-temp_array[i-10]) * onOff_array[i] /100 );  // Low
  analogWrite(high_pins[i-10], dim_amt(lux_array[i]) *     temp_array[i-10]   * onOff_array[i] /100 );  // High  
}


//PHYSICAL:
//device_id/N N=1-10: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=11,12 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
//device_id/ACN: should accept: anything, "on", "off"
// +1 -1 or 0: lt_array:  NA, lm1, lm2, lm3, lm4, lm5, lm6, lm7, lm8, lm9, lm10, lm11Lux, lm12Lux, lm11Temp, lm12Temp
int lt_array []    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



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
  state_json["vG"]        = "amp0,0,20;amp1,0,20;amp2,0,20;amp3,0,20;amp4,0,20;amp5,0,20";
  state_json["vL"]        = "1,16,onOff;1,16,lux;11,16,temp;1,5,AConOff";
  state_json["pL"]        = "1,16,;1,5,AC";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);

  topic = String(device_id)+"/amp0";  
  client.publish(topic.c_str(), String(amp0).c_str());
  topic = String(device_id)+"/amp1";  
  client.publish(topic.c_str(), String(amp1).c_str());
  topic = String(device_id)+"/amp2";  
  client.publish(topic.c_str(), String(amp2).c_str());
  topic = String(device_id)+"/amp3";  
  client.publish(topic.c_str(), String(amp3).c_str());
  topic = String(device_id)+"/amp4";  
  client.publish(topic.c_str(), String(amp4).c_str());
  topic = String(device_id)+"/amp5";  
  client.publish(topic.c_str(), String(amp5).c_str());
}



// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  //device_id/onOffN: should accept: anything, "on", "off"
  //device_id/AConOffN: should accept: anything, "on", "off"
  //device_id/luxN: should accept: 5-100
  //device_id/tempN: should accept: 0-255

  if ((topic.substring(topic.indexOf('/') + 1).indexOf("13") != -1)
     || (topic.substring(topic.indexOf('/') + 1).indexOf("14") != -1)
     || (topic.substring(topic.indexOf('/') + 1).indexOf("15") != -1)
     || (topic.substring(topic.indexOf('/') + 1).indexOf("16") != -1)
     || (topic.substring(topic.indexOf('/') + 1).indexOf("AC") != -1) {
      StaticJsonDocument<200> doc;
      doc["topic"] = topic;
      doc["message"] = msg;

      Wire.beginTransmission(8); // Address of the second ESP32
      serializeJson(doc, Wire);
      Wire.endTransmission();
  }
  
  for (int i = 1; i<=12; i++) {        
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

  //for (int i = 1; i<=3; i++) {        
  //  if (topic == String(device_id) + "/" + "AConOff" + String(i)) {
  //    onOff_array[i+12] = 1-onOff_array[i+12];  // any msg will switch between 1 and 0
  //    if (msg == "on") {
  //      onOff_array[i] = 1;
  //    }
  //    if (msg == "off") {
  //      onOff_array[i] = 0;    
  //    }
  //  }
  //}
  
  for (int i = 1; i<=12; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 11; i<=12; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i-10] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-10 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  
  for (int i = 1; i<=10; i++) {        
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

  //device_id/N N=11,12 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //lm11Lux, lm12Lux, lm11Temp, lm12Temp

  for (int i = 11; i<=12; i++) {        
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

  //device_id/ACN: should accept: anything, "on", "off"  
  //for (int i = 1; i<=3; i++) {        
  //  if (topic == String(device_id) + "/" + String("AC") + String(i)) {
  //    onOff_array[i+12] = 1-onOff_array[i+12];  // any msg will switch between 1 and 0g
  //    if (msg == "on") {
  //      onOff_array[i] = 1;
  //    } 
  //    if (msg == "off") {
  //      onOff_array[i] = 0;    
  //    }
  //  }
  //}

  //Take action from non-sweeping commands, both virtual and physical.
  //Sweeping commands are handled in the main loop.
  for (int i = 1; i<=10; i++) {
    analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }

  for (int i = 11; i<=12; i++) {
    t_write(i);
  }

  //digitalWrite(22, onOff_array[13]);   // AC1
  //digitalWrite(2,  onOff_array[14]);   // AC2
  //digitalWrite(12, onOff_array[15]);   // AC3
  
}


// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  for (int i = 0; i<=5; i++) {        
    topic = String(device_id)+"/"+String("amp")+String(i);
    client.subscribe(topic.c_str());
    }

  for (int i = 1; i<=16; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
    }

  for (int i = 1; i <= 16; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }

  for (int i = 1; i<=16; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 11; i<=16; i++) {        
    client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
    }
    
  for (int i = 1; i<=5; i++) {        
    client.subscribe((String(device_id)+"/"+String("AConOff")+String(i)).c_str());
    }

  for (int i = 1; i<=5; i++) {        
    client.subscribe((String(device_id)+"/"+String("AC")+String(i)).c_str());
    }
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  Wire.begin();
  
  //int temp_AC_pins[] = {33, 32, 21, 23, 22, 2, 12};
  
  //pinMode(A0, INPUT);
 
  for (int i = 1; i<=10; i++) { 
    digitalWrite(mom_pin_array[i], OUTPUT);    //01-10
    digitalWrite(mom_pin_array[i], HIGH);      //01-10
  }
  
  //for (int i = 0; i<=6; i++) { 
  //  digitalWrite(temp_AC_pins[i], OUTPUT);  //Low temp, High temp, and AC pins
  //  digitalWrite(temp_AC_pins[i], HIGH);
  //} 
  
  delay(2000);
  
  for (int i = 1; i<=10; i++) { 
    digitalWrite(mom_pin_array[i], LOW);    //01-10
  }

  //for (int i = 0; i<=6; i++) { 
  //  digitalWrite(temp_AC_pins[i], LOW);    //Low temp, High temp, and AC pins
  //}
}



void take_readings() {
  for (int i = 0; i<=5; i++) {
    voltage_array[i] = analogRead(input_pins[i]) * 3.3 / 4096.0;
    current_array[i] = (voltage_array[i] -1.5) / 0.066;
    sum_sqrd_current_array[i] += current_array[i] * current_array[i];
    //max_current_array[i] = max(max_current_array[i], current_array[i]);
    delay(2);  
  }
}
//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp
void loop() {
  ezama_loop();  //in ezama.h

  String topic {};
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    for (int i = 0; i<=5; i++) {
      sum_sqrd_current_array[i] = 0;
      max_current_array[i] = 0;
      rms_current_array[i] = 0;
      max_current_over_root_2_array[i] = 0; 
    }
    for (int i = 0; i < 500; i++) {
      take_readings();
    }
    for (int i = 0; i<=5; i++) {
      //rms_current_array[i] = max(sqrt(sum_sqrd_current_array[i]/500.0), max_current/sqrt(2));
      rms_current_array[i] = sqrt((1/500.0) * sum_sqrd_current_array[i]);
      //max_current_over_root_2_array[i] = max_current_array[i] / sqrt(2.0);
    }
    for (int i = 0; i<=5; i++) {
      topic = String(device_id)+"/amp" + String(i);  
      client.publish(topic.c_str(), String(rms_current_array[i]).c_str());
      //topic = String(device_id)+"/ampx" + String(i);  
      //client.publish(topic.c_str(), String(max_current_over_root_2_array[i]).c_str());
    }
  }


  
  for(int i=1;i<=10;i++) {
    if(lt_array[i] == 1 && lux_array[i] < 99){   // like increasing the dim slider
        lux_array[i] += 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );    
    }
    if(lt_array[i] == -1 && lux_array[i] > 0){  // like decreasing the dim slider
        lux_array[i] -= 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
    }
  }
  
  for(int i=11;i<=12;i++) {  
    if (lt_array[i] == 1 && lux_array[i] < 99) {  // like increasing the dim slider
       lux_array[i] += 1;
       t_write(i);
    }    
    if(lt_array[i] == -1 && lux_array[i] > 5)  {  // like decreasing the dim slider
        lux_array[i] -= 1;
        t_write(i);
    }
    if(lt_array[i+2] == 1 && temp_array[i-10] < 255)  {   // like increasing the temp slider
        temp_array[i-10] += 5;
        if (temp_array[i-10] >= 255) {temp_array[i-10] = 255;}
        t_write(i);
    }
    if(lt_array[i+2] == -1 && temp_array[i-10] > 0){  // like decreasing the temp slider
        temp_array[i-10] -= 5;
        if (temp_array[i-10] <= 0) {temp_array[i-10] = 0;}
        t_write(i);
    }
  }        

  delay(50);
}
