#include <Ezama10_1.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "BME680 Environmental Sensor";
String ver = "10.0";

Adafruit_BME680 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
float tempC {};
float tempF {};
float hpa {};
float psi {};
float humidPct {};
float airR {};
float a {};


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
  state_json["vG"] = "C,-30,50;F,-20,120;hPa,970,1060;psi,14,15.5;humidPct,0,100;airR,25,300";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.print(sj);

  topic = String(device_id)+"/C";  
  client.publish(topic.c_str(), String(tempC).c_str());
  
  topic = String(device_id)+"/F";  
  client.publish(topic.c_str(), String(tempF).c_str());
  
  topic = String(device_id)+"/hPa";  
  client.publish(topic.c_str(), String(hpa).c_str());
  
  topic = String(device_id)+"/psi";  
  client.publish(topic.c_str(), String(psi).c_str());
  
  topic = String(device_id)+"/humidPct";  
  client.publish(topic.c_str(), String(humidPct).c_str());

  topic = String(device_id)+"/airR";  
  client.publish(topic.c_str(), String(airR).c_str());
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {

}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  
}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms 
}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h

  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());
  
  tempC = bme.temperature;
  tempF = 1.8 * tempC + 32;
  hpa = bme.pressure / 100.0;
  psi = 0.0145038 * hpa;
  humidPct = bme.humidity;
  airR = bme.gas_resistance / 1000.0;
  a = bme.readAltitude(SEALEVELPRESSURE_HPA);  
  
  delay(1000);
}
