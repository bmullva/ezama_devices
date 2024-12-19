// 0 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
  #define MCP23017_SCL 5
  #define MCP23017_SDA 17
  #define RESET_PIN 32  // GPIO32 connected to MCP23017 reset
  bool mcpInitialized = false;
  String mcp_found {};
  
  String type_ = "POE Momentary";
  String ver = "10.3";

// 1 SPECIAL CUSTOM VARIABLES
int d_pin_reading [13]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int d_pin_n1_reading [13]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long startMillis [13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk [13]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel [13]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int mom_pins [13] = {0,1,2,3,4,5,8,9,10,11,12,13};

// Define enums for different states

  
// 2 SPECIAL CUSTOM FUNCTIONS
void initializeMCP() {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) {
        if (mcp0.begin_I2C(0x24)) {
            mcpInitialized = true;
        } else {
            //resetMCP23017();
        }
        lastAttempt = millis();
    }
}

void resetMCP23017() {
    digitalWrite(RESET_PIN, LOW); 
    delay(10);                     
    digitalWrite(RESET_PIN, HIGH);
}

enum ButtonState {
  OFF,
  ON
};
ButtonState timer[4]          = {OFF, OFF, OFF, OFF};



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
  
}

// 4 RECEIVE CONTROLS (to this exact device, from callback, either VIRTUAL or PHYSICAL)
void receive_controls_json(String topic, String msg) {    

}


// 5 SEND CONTROLS (publish_controls only if this is a remote activator)
// Only these will be published: "on", "off", "dim", "brighten", "heat", "cool"
void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  client.publish(topic.c_str(), pin_msg.c_str());
}


// 6 SPECIFIC MQTT SUBSCRIPTIONS (onOff, lux and temp from VIRTUAL UI, sole number from PHYSICAL SWITCH)
void specific_connect() {

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
}


// 8 MAIN LOOP
void custom_loop() {
    if (!mcpInitialized) {
        initializeMCP();
    }
  
  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = mcp0.digitalRead(mom_pins[i]);
  }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 4; i++) {  
    
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
          if (i == 0) {publish_controls("0", "release");}
          if (i == 1) {publish_controls("0", "release");}
          if (i == 2) {publish_controls("1", "release");}
          if (i == 3) {publish_controls("1", "release");}
    }
    
    // Check for a button press (transition from HIGH to LOW) and start a timer
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == OFF) {
      startMillis[i] = millis();
      timer[i] = ON;
    }
  
    // Check for a button release (transition from LOW to HIGH)
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == ON) {
      rel[i] += 1;
    }

    // Check for a button press (transition from HIGH to LOW)
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == ON) {
      clk[i] += 1;
    }
    
    // Check for timer expiration
    if (millis() - startMillis[i] > 500) {
      if (timer[i] == ON) {
        if (clk[i] == 1 && rel[i] == 0) {                     //hold
          if (i == 0) {publish_controls("0", "dim");}
          if (i == 1) {publish_controls("0", "brighten");}
          if (i == 2) {publish_controls("1", "dim");}
          if (i == 3) {publish_controls("1", "brighten");}
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          if (i == 0) {publish_controls("0", "off");}
          if (i == 1) {publish_controls("0", "on");}
          if (i == 2) {publish_controls("1", "off");}
          if (i == 3) {publish_controls("1", "on");}   
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("0", "cool");}
          if (i == 1) {publish_controls("0", "heat");}
          if (i == 2) {publish_controls("1", "cool");}
          if (i == 3) {publish_controls("1", "heat");} 
        } //else if (clk[i] == 2 && rel[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("0", "??");}
         // if (i == 1) {publish_controls("0", "??");}
         // if (i == 2) {publish_controls("1", "??");}
         // if (i == 3) {publish_controls("1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk[i] = 0;
      rel[i] = 0;
      timer[i] = OFF;
    }
  
    // Update the previous reading for the next iteration
    d_pin_n1_reading[i] = d_pin_reading[i];
  }

  delay(100);
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
