#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40

// Connect the INA226 sensor to the ESP32 according to the wiring diagram below.
// SDA -> ESP32 SDA
// SCL -> ESP32 SCL
// VCC -> 3.3V
// GND -> GND
// INA226 Internal Connections
// VBUS -> IN+
// GND -> IN-

INA226_WE ina226(I2C_ADDRESS);

void setup() {
  Serial.begin(115200);
  if (!Wire.begin()) {
    Serial.println("I2C initialization failed!");
    while (1); // Stop here if I2C initialization fails
  }
  ina226.init();
  Serial.println("INA226 Current Sensor Example Sketch");
  Serial.println("Continuous Sampling starts");
  Serial.println();
}

void loop() {
  continuousSampling();
  delay(3000); // 3 seconds delay between samples
}

void continuousSampling(){
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 

  ina226.readAndClearFlags();
  shuntVoltage_mV = ina226.getShuntVoltage_mV();
  busVoltage_V = ina226.getBusVoltage_V();
  current_mA = ina226.getCurrent_mA();
  power_mW = ina226.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV / 1000);
  
  Serial.print("Shunt Voltage [mV]: "); Serial.println(shuntVoltage_mV);
  Serial.print("Bus Voltage [V]: "); Serial.println(busVoltage_V);
  Serial.print("Load Voltage [V]: "); Serial.println(loadVoltage_V);
  Serial.print("Current [mA]: "); Serial.println(current_mA);
  Serial.print("Bus Power [mW]: "); Serial.println(power_mW);
  
  if (!ina226.overflow){
    Serial.println("Values OK - no overflow");
  } else {
    Serial.println("Overflow! Choose higher current range");
  }
  Serial.println();
}
