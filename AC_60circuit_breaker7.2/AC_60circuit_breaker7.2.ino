#include <Ezama.h>  // This is for a DOIT ESP32 DEVKIT V1 with 30 pins
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "AC 60 Circuit";
String ver = "7.2";

float Amps1_TRMS {};
float Amps2_TRMS {};
float Amps3_TRMS {};
float Amps4_TRMS {};
float Amps5_TRMS {};
float Amps6_TRMS {};

float amphour1 = 0;
float amphour2 = 0;
float amphour3 = 0;
float amphour4 = 0;
float amphour5 = 0;
float amphour6 = 0;

float testFrequency = 60;
float windowLength = 40.0/testFrequency;
float b1 = -8938.656737;
float b2 = 612.927984;

unsigned long printPeriod = 2000; // in milliseconds
unsigned long previousMillis = 0;

float s2a(float x) {
  float newx = 3.3 * x / 4024.0;
  float amp = b1*newx + b2*newx*newx;
  return amp;
}


// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String topic = "reporting";
  String output1;
  String output2;
  String output3;
  String output4;
  DynamicJsonDocument state_json1(1024);
  DynamicJsonDocument state_json2(1024);
  DynamicJsonDocument state_json3(1024);
  DynamicJsonDocument state_json4(1024);
  char sj1[1024];
  char sj2[1024];
  char sj3[1024];
  char sj4[1024];
  state_json1["device_id"] = device_id;
  state_json1["descript"] = descript;
  state_json1["type"] = type;
  state_json1["ver"] = ver;
  state_json1["IP"] = WiFi.localIP();
  
  state_json2["device_id"] = device_id;
  state_json2["A1"] = Amps1_TRMS;
  state_json2["A2"] = Amps2_TRMS;
  state_json2["A3"] = Amps3_TRMS;
  state_json2["A4"] = Amps4_TRMS;
  state_json2["A5"] = Amps5_TRMS;
  state_json2["A6"] = Amps6_TRMS;
  
  state_json3["device_id"] = device_id;
  state_json3["Ah1"] = amphour1;
  state_json3["Ah2"] = amphour2;
  state_json3["Ah3"] = amphour3;
  state_json3["Ah4"] = amphour4;
  state_json3["Ah5"] = amphour5;
  state_json3["Ah6"] = amphour6;
  
  state_json4["device_id"] = device_id;
  state_json4["slg1"] = (digitalRead(23) == LOW) ? "on" : "off";
  state_json4["slg2"] = (digitalRead(22) == LOW) ? "on" : "off";
  state_json4["slg3"] = (digitalRead(21) == LOW) ? "on" : "off";
  state_json4["slg4"] = (digitalRead(19) == LOW) ? "on" : "off";
  state_json4["slg5"] = (digitalRead(17) == LOW) ? "on" : "off";
  state_json4["slg6"] = (digitalRead(16) == LOW) ? "on" : "off";
  
  serializeJson(state_json1, output1);
  serializeJson(state_json2, output2);
  serializeJson(state_json3, output3);
  serializeJson(state_json4, output4);
  output1.toCharArray(sj1, 1024);
  output2.toCharArray(sj2, 1024);
  output3.toCharArray(sj3, 1024);
  output4.toCharArray(sj4, 1024);
  publishMessage(topic.c_str(), sj1);
  publishMessage(topic.c_str(), sj2);
  publishMessage(topic.c_str(), sj3);
  publishMessage(topic.c_str(), sj4);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["cat"] = "sensor,actuator";  // sensor, controller, actuator
  state_json["power_switch"]= "0,1";
  state_json["cmds"] = "slg channel:cmd";
  state_json["ex"] = "on BOARD_ID topic, slg3:on";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  //client.publish(topic.c_str(), sj);
  publishMessage(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {
   
  StaticJsonDocument<256> jdoc;    
  DeserializationError error = deserializeJson(jdoc, messageTemp); // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String slg1_msg = jdoc["slg1"];
  String slg2_msg = jdoc["slg2"];
  String slg3_msg = jdoc["slg3"];
  String slg4_msg = jdoc["slg4"];
  String slg5_msg = jdoc["slg5"];
  String slg6_msg = jdoc["slg6"];

  if(slg1_msg == "on") {digitalWrite(23, LOW);}
  if(slg2_msg == "on") {digitalWrite(22, LOW);}
  if(slg3_msg == "on") {digitalWrite(21, LOW);}
  if(slg4_msg == "on") {digitalWrite(19, LOW);}
  if(slg5_msg == "on") {digitalWrite(17, LOW);}
  if(slg6_msg == "on") {digitalWrite(16, LOW);}
  
  if(slg1_msg == "off") {digitalWrite(23, HIGH);}
  if(slg2_msg == "off") {digitalWrite(22, HIGH);}
  if(slg3_msg == "off") {digitalWrite(21, HIGH);}
  if(slg4_msg == "off") {digitalWrite(19, HIGH);}
  if(slg5_msg == "off") {digitalWrite(17, HIGH);}
  if(slg6_msg == "off") {digitalWrite(16, HIGH);}

}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(23, OUTPUT); // SLG 1
  pinMode(22, OUTPUT); // SLG 2
  pinMode(21, OUTPUT); // SLG 3
  pinMode(19, OUTPUT); // SLG 4
  pinMode(17, OUTPUT); // SLG 5
  pinMode(16, OUTPUT); // SLG 6
  
  pinMode(36, INPUT); // VOG 1
  pinMode(39, INPUT); // GOV 2
  pinMode(34, INPUT); // GOV 3
  pinMode(35, INPUT); // VOG 4
  pinMode(32, INPUT); // VOG 5
  pinMode(33, INPUT); // VOG 6
  
  digitalWrite(23, LOW); // SLG 1
  digitalWrite(22, LOW); // SLG 2
  digitalWrite(21, LOW); // SLG 3
  digitalWrite(19, LOW); // SLG 4
  digitalWrite(17, LOW); // SLG 5
  digitalWrite(16, LOW); // SLG 6
}


//7 MAIN LOOP
void loop() {
  ezama_loop();
  
  if (setting == 'n') {
    RunningStatistics stats1;           // create raw test signal stats 
    RunningStatistics stats2;           // create raw test signal stats
    RunningStatistics stats3;           // create raw test signal stats
    RunningStatistics stats4;           // create raw test signal stats
    RunningStatistics stats5;           // create raw test signal stats
    RunningStatistics stats6;           // create raw test signal stats

    stats1.setWindowSecs( windowLength );     //Set window length
    stats2.setWindowSecs( windowLength );     //Set window length
    stats3.setWindowSecs( windowLength );     //Set window length
    stats4.setWindowSecs( windowLength );     //Set window length
    stats5.setWindowSecs( windowLength );     //Set window length
    stats6.setWindowSecs( windowLength );     //Set window length

    while( true ) { 
      ezama_loop();  //in Ezama.h
      
      if (millis() % 17 == 0) {
        stats1.input(analogRead(36));
        stats2.input(analogRead(39));
        stats3.input(analogRead(34));
        stats4.input(analogRead(35));
        stats5.input(analogRead(32));
        stats6.input(analogRead(33));
      }

      if((unsigned long)(millis() - previousMillis) >= printPeriod) { //every second we do the calculation
        previousMillis = millis();   // update time

        Amps1_TRMS = s2a(stats1.sigma());
        Amps2_TRMS = s2a(stats2.sigma());
        Amps3_TRMS = s2a(stats3.sigma());
        Amps4_TRMS = s2a(stats4.sigma());
        Amps5_TRMS = s2a(stats5.sigma());
        Amps6_TRMS = s2a(stats6.sigma());
        
        amphour1 += Amps1_TRMS * 2/3600;
        amphour2 += Amps2_TRMS * 2/3600;
        amphour3 += Amps3_TRMS * 2/3600;
        amphour4 += Amps4_TRMS * 2/3600;
        amphour5 += Amps5_TRMS * 2/3600;
        amphour6 += Amps6_TRMS * 2/3600;
      }
    }
    delay(50);
  }
}
