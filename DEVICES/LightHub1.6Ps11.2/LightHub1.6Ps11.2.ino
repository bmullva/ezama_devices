#include <Wire.h>    // For ESP-32 DOIT ESP23
#include <ArduinoJson.h>

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/AConOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 5-100
//device_id/tempN: should accept: 0-255
// onOff_array: onOff 0, 1: {NA, 13, 14, 15, 16, AC1, AC2, AC3, AC4, AC5};
// lux_array:  lux 0-100 {NA, lux13, lux14, lux15, lux16};
// temp_array:  {temp13, temp14, temp15, temp16};
int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0, 0};
int temp_array[]   = {-99, 0, 0, 0, 0};


//int mom_pin_array[]= {-99,13,4,14,16,27,17,26,18,25,19};
// AC PINS: 17, 16, 4, 18, 19
int low_pins[]  = {-99, 32, 12, 25, 27};
int high_pins[] = {-99, 33, 13, 26, 14};
void t_write(int i) {
  analogWrite(low_pins[i],  dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  analogWrite(high_pins[i], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High  
}

//PHYSICAL:
//device_id/N N=1-10: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=11,12 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
//device_id/ACN: should accept: anything, "on", "off"
// +1 -1 or 0: lt_array:  NA, lm13Lux, lm14Lux, lm15Lux, lm16Lux, lm13Temp, lm14Temp, lm15Temp, lm16Temp
int lt_array []    = {-99, 0, 0, 0, 0, 0, 0, 0, 0};


// 2 REPORT (SENT EVERY 6 SECONDS)


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receiveEvent(int numBytes) {
  StaticJsonDocument<200> doc;
  
  while (Wire.available()) {
    deserializeJson(doc, Wire);
  }
  String topic = doc["topic"];
  String msg = doc["message"];

  for (int i = 13; i<=16; i++) {        
    if (topic == topic.substring(0, 8) + "/" + String("onOff") + String(i)) {
      onOff_array[i-12] = 1-onOff_array[i-12];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i-12] = 1;
      } 
      if (msg == "off") {
        onOff_array[i-12] = 0;    
      }
    }
  }

  for (int i = 1; i<=5; i++) {        
    if (topic == topic.substring(0, 8) + "/" + "AConOff" + String(i)) {
      onOff_array[i+4] = 1-onOff_array[i+4];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i+4] = 1;
      }
      if (msg == "off") {
        onOff_array[i+4] = 0;    
      }
    }
  }
  
  for (int i = 13; i<=16; i++) {        
    if (topic == topic.substring(0, 8) + "/" + "lux" + String(i)) {
      lux_array[i-12] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 13; i<=16; i++) {        
    if (topic == topic.substring(0, 8) + "/" + "temp" + String(i)) {
      temp_array[i-12] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-10 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  
  //for (int i = 1; i<=10; i++) {        
  //  if (topic == String(device_id) + "/" + String(i)) {

  //      if (msg == "hold") {
  //        lt_array[i] = 1;
  //      }
        
  //      if (msg == "click") {
  //        onOff_array[i] = 1 - onOff_array[i];
  //      }

  //      if (msg == "click-hold") {
  //        lt_array[i] = -1;
  //      }
        
  //      if (msg == "dbl-click") {
  //        
  //      }

  //      if (msg == "on") {
  //        onOff_array[i] = 1;
  //      }
        
  //      if (msg == "off") {
  //        onOff_array[i] = 0;    
  //      }

  //      if (msg == "dim") {
  //        lt_array[i] = 1;
  //      }

  //      if (msg == "brighten") {
  //        lt_array[i] = -1;
  //      }
        
  //      if (msg == "release") {
  //        lt_array[i] = 0;
  //      }        
  //  }
  //}

  //device_id/N N=13,16 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //lm11Lux, lm12Lux, lm11Temp, lm12Temp

  for (int i = 13; i<=16; i++) {        
    if (topic == topic.substring(0, 8) + "/" + String(i)) {
        // msg to these topics should be "click", "on", "off", "dim", "brighten", "heat", "cool", "release"
               
        if (msg == "click") {
          onOff_array[i-12] = 1 - onOff_array[i];
        }
        
        if (msg == "on") {
          onOff_array[i-12] = 1;
        }

        if (msg == "off") {
          onOff_array[i-12] = 0;
        }
        
        if (msg == "dim") {
          lt_array[i-12] = 1;
        }

        if (msg == "brighten") {
          lt_array[i-12] = -1;
        }

         if (msg == "heat") {
          lt_array[i-8] = 1;
        }
        
        if (msg == "cool") {
          lt_array[i-8] = -1;
        }

        if (msg == "release") {
          lt_array[i-12] = 0;
          lt_array[i-8] = 0;
        }
    }
  }

  //device_id/ACN: should accept: anything, "on", "off"  
  for (int i = 1; i<=5; i++) {        
    if (topic == topic.substring(0, 8) + "/" + String("AC") + String(i)) {
      onOff_array[i+4] = 1-onOff_array[i+4];  // any msg will switch between 1 and 0g
      if (msg == "on") {
        onOff_array[i+4] = 1;
      } 
      if (msg == "off") {
        onOff_array[i+4] = 0;    
      }
    }
  }

  //Take action from non-sweeping commands, both virtual and physical.
  //Sweeping commands are handled in the main loop.
  //for (int i = 1; i<=10; i++) {
  //  analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  //}

  for (int i = 1; i<=4; i++) {
    t_write(i);
  }

  digitalWrite(17, onOff_array[5]);   // AC1
  digitalWrite(16,  onOff_array[6]);   // AC2
  digitalWrite(4, onOff_array[7]);   // AC3
  digitalWrite(18,  onOff_array[8]);   // AC4
  digitalWrite(19, onOff_array[9]);   // AC5
}


// 5 SEND CONTROLS (publish_controls only if controller module)
//void publish_controls_json(String pin_name, String pin_msg) {

//}


//6 SETUP (pins)
void setup() { 
  Wire.begin(8);                // Address of this ESP32
  Wire.onReceive(receiveEvent); // Register receive event

  int temp_AC_pins[] = {17, 16, 4, 18, 19, 32, 12, 25, 27, 33, 13, 26, 14};
  
  for (int i = 0; i<=12; i++) { 
    pinMode(temp_AC_pins[i], OUTPUT);  //Low temp, High temp, and AC pins
    digitalWrite(temp_AC_pins[i], HIGH);
  } 
  
  delay(2000);
  
  for (int i = 0; i<=12; i++) { 
    digitalWrite(temp_AC_pins[i], LOW);    //Low temp, High temp, and AC pins
  }
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  
    for(int i=1;i<=4;i++) {  
    if (lt_array[i] == 1 && lux_array[i] < 99) {  // like increasing the dim slider
       lux_array[i] += 1;
       t_write(i);
    }    
    if(lt_array[i] == -1 && lux_array[i] > 5)  {  // like decreasing the dim slider
        lux_array[i] -= 1;
        t_write(i);
    }
    if(lt_array[i+4] == 1 && temp_array[i] < 255)  {   // like increasing the temp slider
        temp_array[i] += 5;
        if (temp_array[i] >= 255) {temp_array[i] = 255;}
        t_write(i);
    }
    if(lt_array[i+4] == -1 && temp_array[i] > 0){  // like decreasing the temp slider
        temp_array[i] -= 5;
        if (temp_array[i] <= 0) {temp_array[i] = 0;}
        t_write(i);
    }
  }

  delay(50);
}
