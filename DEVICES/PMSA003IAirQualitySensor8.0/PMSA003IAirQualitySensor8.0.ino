#include <Ezama2.h>
#include <Wire.h>
#include "Adafruit_PM25AQI.h"

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "PMSA003I Air Quality Sensor";
String ver = "8.0";
//Guideline: PM2.5 not exceed 5 μg/m3 annual mean, or 15 μg/m3 24-hour mean
//PM10 not exceed 15 μg/m3 annual mean, or 45 μg/m3 24-hour mean
//Use environmental values, standard are used for calibration
//PM10 is not directly measured, but rather estimated
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
int pm25_env {};
int pm100_env {};



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
  state_json["pm25_env"] = pm25_env;
  state_json["pm100_env"] = pm100_env;
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = device_id + "/ppm1";  
  client.publish(topic.c_str(), ppm1);
  topic = device_id + "ppm2"
  client.publish(topic.c_str(), ppm2);
  
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

}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h
  
  if (! aqi.begin_I2C()) {
    Serial.println(F("Could not find PM 2.5 sensor!"));
    while (1) delay(10);
  }

}


// MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  PM25_AQI_Data data;
  
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  
  //Standard is for factory calibration
  //Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  //Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  //Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  //pm10_env = data.pm10_env;
  pm25_env = data.pm25_env;
  pm100_env = data.pm100_env;
  //Serial.print(F("Particles > 0.3um / 0.1L air:")); 
  //Serial.println(data.particles_03um);
  //Serial.print(F("Particles > 0.5um / 0.1L air:")); 
  //Serial.println(data.particles_05um);
  //Serial.print(F("Particles > 1.0um / 0.1L air:")); 
  //Serial.println(data.particles_10um);
  //Serial.print(F("Particles > 2.5um / 0.1L air:")); 
  //Serial.println(data.particles_25um);
  //Serial.print(F("Particles > 5.0um / 0.1L air:")); 
  //Serial.println(data.particles_50um);
  //Serial.print(F("Particles > 10 um / 0.1L air:")); 
  //Serial.println(data.particles_100um);
  
  delay(1000);
  
}
