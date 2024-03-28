#include <Ezama2.h>  // For ESP8266  MAYBE NEVER USED?
//#include <Filters.h> this should be DC?


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "ACS712 20A Power Flow Sensor";
String ver = "8.0";

const char* a_pin_name = "watts";
float amp_hour = 0;
float ACS_Value;                              //Here we keep the raw data valuess
float testFrequency = 60;                    // test signal frequency (Hz)
float windowLength = 40.0/testFrequency;     // how long to average the signal, for statistist
float intercept = -0.0636; // to be adjusted based on calibration testing
float slope = 0.0637; // to be adjusted based on calibration testing
float sqd_slope = -0.0000754; 
//Please check the ACS712 Tutorial video by SurtrTech to see how to get them because it depends on your sensor, or look below
float Amps_TRMS = 0; // estimated actual current in amps
unsigned long printPeriod = 2000; // in milliseconds
// Track time in milliseconds since last reading 
unsigned long previousMillis = 0;


// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["ver"] = ver;
  state_json["IP"] = WiFi.localIP();
  state_json["A"] = Amps_TRMS;
  state_json["Ah"] = amp_hour;
  if (digitalRead(13) == 0) {
    state_json["power_switch"] = "on";
  }
  if (digitalRead(13) == 1) {
    state_json["power_switch"] = "off";
  }
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = device_id+"/amp";  
  client.publish(topic.c_str(), amp);
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  
  state_json["vGage"] = "[[\"ppm1\", 0, 100000],[\"ppm2\", 0, 100000]]";
  //state_json["vLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pLight"] = "[['start#', 'end#', 'name']]";
  //state_json["pPwtch"] = "[['start#', 'end#', 'name']]";
  
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {
   
   if(messageTemp == "on") {
    digitalWrite(13, LOW);
    Serial.print("Writing 13 to LOW");
   }

   if(messageTemp == "off") {
    digitalWrite(13, HIGH);
    Serial.print("Writing 13 to HIGH");
   } 
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  pinMode(A0, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

}


//7 MAIN LOOP
void loop() {
  ezama_loop();
  
  if (setting == 'n') {
    RunningStatistics inputStats;              // create test signal stats 
    inputStats.setWindowSecs( windowLength );  //Set window length
  
    while( true ) { 
      ezama_loop();  //in ezama.h
      
      if (millis() % 13 == 0) {
        ACS_Value = analogRead(A0);  // read the analog in value:
        inputStats.input(ACS_Value);  // log to Stats function
      }

      if((unsigned long)(millis() - previousMillis) >= printPeriod) { //calc ev 2 sec
        previousMillis = millis();   // update time      
        Amps_TRMS = intercept + slope * inputStats.sigma() + sqd_slope * inputStats.sigma() * inputStats.sigma();
        if(Amps_TRMS < 0) {
          Amps_TRMS = 0.0;
        }
  
        amp_hour += Amps_TRMS * 2/3600;  // 2 sec pritnPeriod
      }
    }
  }
}
