#include <Ezama12.h> // NodeMCU 1.0
const double dVCC = 3.3;             // NodeMCU on board 3.3v vcc
const double dR2 = 10000;            // 10k ohm series resistor
const double dAdcResolution = 1023;  // 10-bit adc

const double dA = 0.001129148;       // thermistor equation parameters
const double dB = 0.000234125;
const double dC = 0.0000000876741; 

double dVout, dRth, dTemperature, dAdcValue;
int setpoint {};

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Temp Cntrl";
String ver = "10.3";


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
  state_json["vG"]        = "setpoint,60,100; tempF,-20,200";
  //state_json["vL"]        = "0,1,set_temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]        = "1,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);

  topic = String(device_id)+"/tempF";  
  client.publish(topic.c_str(), String(dTemperature).c_str());

  topic = String(device_id)+"/setpoint";  
  client.publish(topic.c_str(), String(setpoint).c_str());
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  if (topic == String(device_id) + "/" + String("set_temp")) {
    setpoint = msg.toInt();
    if (setpoint != EEPROM.read(220)) {
      EEPROM.write(220, setpoint);
      EEPROM.commit();
    }
  }
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
// No controls to be sent.
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {

}

void setup() { 
  Serial.begin(115200);
  EEPROM.begin(512);
  ezama_setup();  //in ezama8.h
  client.subscribe((String(device_id) + String("/set_temp")).c_str());
  setpoint = EEPROM.read(220);
}


//7 MAIN LOOP
void loop() {
  ezama_loop();  // Call a function named ezama_loop from ezama.h

  dAdcValue = analogRead(A0);
  dVout = (dAdcValue * dVCC) / dAdcResolution;
  dRth = (dVCC * dR2 / dVout) - dR2;

  //  Steinhart-Hart Thermistor Equation:
  //  Temperature in Kelvin = 1 / (A + B[ln(R)] + C[ln(R)]^3)
  //  where A = 0.001129148, B = 0.000234125 and C = 8.76741*10^-8
  // Temperature in kelvin
  dTemperature = (1 / (dA + (dB * log(dRth)) 
    + (dC * pow((log(dRth)), 3))));   

  // Temperature in degree celsius
  dTemperature = dTemperature - 273.15; 
  dTemperature = (dTemperature * 9/5) + 32;
  Serial.print("Temperature = ");
  Serial.print(dTemperature);
  Serial.println(" degree F");

  if (dTemperature < setpoint) {
    Serial.println("Turned On");
    client.publish(String(device_id).c_str(), "on");
  }
  if (dTemperature > setpoint+3) {
    Serial.println("Turned Off");
    client.publish(String(device_id).c_str(), "off");
  }
  
  delay(1000);
}
