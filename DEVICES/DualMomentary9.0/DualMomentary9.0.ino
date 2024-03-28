#include <Ezama9.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Dual Momentary";
String ver = "9.0";

int d_pin_reading [4]         = {HIGH, HIGH, HIGH, HIGH};
int d_pin_n1_reading [4]      = {HIGH, HIGH, HIGH, HIGH};
unsigned long startMillis [4] = {0, 0, 0, 0};
int clk [4]                   = {0, 0, 0, 0};
int rel [4]                   = {0, 0, 0, 0};

// Define enums for different states
enum ButtonState {
  OFF,
  ON
};
ButtonState timer[4]          = {OFF, OFF, OFF, OFF};


// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = WiFi.localIP();
  state_json["pS"]= "0,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.println(sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
// Only these will be published: "on", "off", "dim", "brighten", "heat", "cool"

void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  client.publish(topic.c_str(), pin_msg.c_str());
}

//void publish_json(String pin_name, String pin_msg) {
// String output;
//  DynamicJsonDocument state_json(1024);
//  char sj[1024];
//  String topic = device_id;
//  state_json[pin_name] = pin_msg;
//  serializeJson(state_json, output);
//  output.toCharArray(sj, 1024);
//  client.publish(topic.c_str(), sj);
//}

//6 SETUP (pins)
void specific_connect() {
  
}

void setup() { 
  ezama_setup();  //in ezama8.h
  
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

}


//7 MAIN LOOP
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

void loop() {
  ezama_loop();  // Call a function named ezama_loop from ezama.h
  
  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(i);
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
