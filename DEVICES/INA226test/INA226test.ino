#include <Ezama12.h>  // For ESP-32 DOIT ESP23
#include <Filters.h>
#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40

// Connect the INA226 sensor to the ESP32 according to the wiring diagram below.
// https://github.com/izzarzn/QuantumVolt
// SDA -> ESP32 SDA
// SCL -> ESP32 SCL
// VCC -> 3.3V
// GND -> GND
// INA226 Internal Connections
// VBUS -> IN+



// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Light Hub 1.6 Test";
String ver = "00INA226";

INA226_WE ina226(I2C_ADDRESS);
float shuntVoltage_mV = 0.0;
float loadVoltage_V = 0.0;
float busVoltage_V = 0.0;
float current_mA = 0.0;
float power_mW = 0.0;
int i = 0;
String topic {};
int call_count = 0; // Add this line at the top to declare the counter

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
  //state_json["vL"]        = "1,13,onOff;1,13,lux;11,13,temp";
  //state_json["pL"]        = "1,13,";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);

  ina226.readAndClearFlags();
  shuntVoltage_mV = ina226.getShuntVoltage_mV();
  busVoltage_V = ina226.getBusVoltage_V();
  current_mA = ina226.getCurrent_mA();
  power_mW = ina226.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV / 1000);

  analogWrite(15,i);
  call_count++; // Increment the call counter

  if (call_count % 2 == 0) { // Increment `i` every other call
    i += 10;
    if (i >= 255) {
      i = 0;
    }
  }

  topic = String(device_id)+"/shuntVoltage_mV";  
  client.publish(topic.c_str(), String(shuntVoltage_mV).c_str());
  topic = String(device_id)+"/busVoltage_V";
  client.publish(topic.c_str(), String(busVoltage_V).c_str());
  topic = String(device_id)+"/loadVoltage_V";
  client.publish(topic.c_str(), String(loadVoltage_V).c_str());
  topic = String(device_id)+"/current_mA";
  String a = String(i)+" - "+String(current_mA);
  client.publish(topic.c_str(), a.c_str());
  topic = String(device_id)+"/power_mW";
  client.publish(topic.c_str(), String(power_mW).c_str());
}



// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {

}


// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  
  topic = String(device_id)+"/shuntVoltage_mV";  
  client.subscribe(topic.c_str());

  topic = String(device_id)+"/busVoltage_V";
  client.subscribe(topic.c_str());
  
  topic = String(device_id)+"/loadVoltage_V";
  client.subscribe(topic.c_str());
  
  topic = String(device_id)+"/current_mA";
  client.subscribe(topic.c_str());
  
  topic = String(device_id)+"/power_mW";
  client.subscribe(topic.c_str());

}

void setup() { 
  //Serial.begin(115200);
  ezama_setup();  //in ezama.h
  specific_connect();
  //pinMode(4, OUTPUT);
  pinMode(15, OUTPUT);
  analogWrite(15,255);
  delay(2000);
  analogWrite(15,0);
  
  Wire.begin();
  ina226.init();

  float shunt_resistor_value = 0.002; // 0.002 ohms
  float max_current_range = 40.0; // 40A
  //maybe with 20A?

  ina226.setResistorRange(shunt_resistor_value, max_current_range);
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h

  delay(50);
}
