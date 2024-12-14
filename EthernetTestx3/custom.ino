// 0 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
  #define MCP23017_SCL 5
  #define MCP23017_SDA 17
  #define RESET_PIN 32  // GPIO32 connected to MCP23017 reset
  bool mcpInitialized = false;
  String mcp_found {};
  
  String type_ = "Ethernet Light Hub";
  String ver = "2.0";

// 1 SPECIAL CUSTOM VARIABLES
  //VIRTUAL VARIABLES FOR NODE RED UI:
  //from device_id/onOffN: should accept: anything, "on", "off"
  //from device_id/luxN: should accept: 0-95
  //from device_id/tempN: should accept: 0-255

  // onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  // lux_array:  lux 0-100 {NA,lux1,lux2,lux3,lux4,lux5,lux6,lux7,lux8,lux9,lux10,lux11,lux12,lux13,lux14};
  // temp_array:  {NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,temp13,temp14};
  int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int temp_array[]   = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 0, 0};

  //mom_pin_array: these are GPIOs (1-12) for the Single Color Lights
  //low_pins and high_pins are the GPIOs (13-14) for high & low temp lights
  int mom_pin_array[]= {-99,15,0,14,1,13,2,12,3,11,4,10,5};
  int low_pins[]  = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 9, 6};
  int high_pins[] = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 8, 7};

  //PHYSICAL VARIABLES FOR SWITCHES:
  // device_id/N N=1-12 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  // device_id/N (N=13-14) should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
  // lt_array (+1 -1 or 0): NA, lm1,lm2,lm3,lm4,lm5,lm6,lm7,lm8,lm9,lm10,lm11,lm12,lux13,lux14,temp13,temp14 
  int lt_array [] = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
// 2 SPECIAL CUSTOM FUNCTIONS
void initializeMCP() {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) {
        if (mcp0.begin_I2C(0x20)) {
            mcpInitialized = true;
        } else {
            resetMCP23017();
        }
        lastAttempt = millis();
    }
}

void resetMCP23017() {
    digitalWrite(RESET_PIN, LOW); 
    delay(10);                     
    digitalWrite(RESET_PIN, HIGH);
}

int dim_amt (int dim) {
  return 100 - dim;
}

void mcp0_analogWrite(int pin, int value) {
  for (int i = 0; i < 255; i++) {
    if (i < value) {
      mcp0.digitalWrite(pin, HIGH);
    } else {
      mcp0.digitalWrite(pin, LOW);
    }
    delayMicroseconds(25);  // Adjust this value for desired PWM frequency
  }
}

void t_write(int i) {
  mcp0_analogWrite(low_pins[i],  dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  mcp0_analogWrite(high_pins[i], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High  
}



// 3 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  state_json["MCP"] = mcp_found;
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
  
  //serializeJson(state_json, output);
  //output.toCharArray(sj, 1024);
  //client.publish(topic.c_str(), sj);
}

// 4 RECEIVE CONTROLS (to this exact device, from callback, either VIRTUAL or PHYSICAL)
void receive_controls_json(String topic, String msg) {    
  // VIRTUAL MSGS: 
  // device_id/onOffN: should accept: anything, "on", "off"
  // device_id/luxN: should accept: 0-95
  // device_id/tempN: should accept: 0-255

  String debugMsg = "receive controls - " + topic + " " + msg;
  client.publish("debug", debugMsg.c_str());
  for (int i = 1; i<=14; i++) {        
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
  
  for (int i = 1; i<=14; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 13; i<=14; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MSGS
  //device_id/N N=1-12 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  //device_id/N N=13-14 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //NA, lux13, lux14, temp13, temp14

  for (int i = 1; i<=12; i++) {        
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

  for (int i = 13; i<=14; i++) {        
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

  // TAKE ACTION on onOff_array, lux_array, and temp_array 
  // Sweeping" commands are handled in the main_loop
  // Sweeping commands includes "dim", "brighten", "heat", "cool", "release"
  for (int i = 1; i<=12; i++) {
    String debugMsg = String(mom_pin_array[i]) + ":" + String(dim_amt(lux_array[i]) * 255 * onOff_array[i] / 100);
    client.publish("debug", debugMsg.c_str());
    mcp0_analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }
  for (int i = 13; i<=14; i++) {
    t_write(i);
  }
}


// 5 SEND CONTROLS (publish_controls only if this is a remote activator)
void publish_controls(String switch_num, String pin_msg) {

}


// 6 SPECIFIC MQTT SUBSCRIPTIONS (onOff, lux and temp from VIRTUAL UI, sole number from PHYSICAL SWITCH)
void specific_connect() {
  String topic {};
  for (int i = 1; i <= 14; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }
  for (int i = 1; i<=14; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
  } 
  for (int i = 13; i<=14; i++) {        
    client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
  }
  for (int i = 1; i<=14; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
  }
  client.publish("reporting", "I have subscribed to the topics");
  client.publish("debug", "I have subscribed to the topics");
}


// 7 SETUP
void custom_setup() { 
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); // Keep reset high by default
  Wire.begin(MCP23017_SDA, MCP23017_SCL);
  initializeMCP();
  if (mcpInitialized == false) {
    client.publish("debug", "MCP NOT INITIALIZED");
  }
  else {
    client.publish("debug", "MCP INITIALIZED");
  }
  mcp0.pinMode(0, OUTPUT);
  mcp0.pinMode(1, OUTPUT);
  mcp0.pinMode(2, OUTPUT);
  mcp0.pinMode(3, OUTPUT);
  mcp0.pinMode(4, OUTPUT);
  mcp0.pinMode(5, OUTPUT);
  mcp0.pinMode(8, OUTPUT);
  mcp0.pinMode(9, OUTPUT);
  mcp0.pinMode(10, OUTPUT);
  mcp0.pinMode(11, OUTPUT);
  mcp0.pinMode(12, OUTPUT);
  mcp0.pinMode(13, OUTPUT);
  mcp0.pinMode(14, OUTPUT);
  mcp0.pinMode(15, OUTPUT);
}


// 8 MAIN LOOP
void custom_loop() {
    if (!mcpInitialized) {
        initializeMCP();
    }
  
  for(int i=1;i<=12;i++) {
    if(lt_array[i] == 1 && lux_array[i] < 99){   // like increasing the dim slider
        lux_array[i] += 1;
        mcp0_analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );    
    }
    if(lt_array[i] == -1 && lux_array[i] > 0){  // like decreasing the dim slider
        lux_array[i] -= 1;
        mcp0_analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
    }
  }
  
  for(int i=13;i<=14;i++) {  
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



/*
const int GPA0 = 0;   // GPIOA pin 0
const int GPA1 = 1;   // GPIOA pin 1
const int GPA2 = 2;   // GPIOA pin 2
const int GPA3 = 3;   // GPIOA pin 3
const int GPA4 = 4;   // GPIOA pin 4
const int GPA5 = 5;   // GPIOA pin 5
const int GPA6 = 6;   // GPIOA pin 6
const int GPA7 = 7;   // GPIOA pin 7

const int GPB0 = 8;   // GPIOB pin 0
const int GPB1 = 9;   // GPIOB pin 1
const int GPB2 = 10;  // GPIOB pin 2
const int GPB3 = 11;  // GPIOB pin 3
const int GPB4 = 12;  // GPIOB pin 4
const int GPB5 = 13;  // GPIOB pin 5
const int GPB6 = 14;  // GPIOB pin 6
const int GPB7 = 15;  // GPIOB pin 7

A2 A1 A0 
000 = 0x20
001 = 0x21
010 = 0x22
011 = 0x23
100 = 0x24
101 = 0x25
110 = 0x26
111 = 0x27
VSS = GND
VDD = 5V
*/
