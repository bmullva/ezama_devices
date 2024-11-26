#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial monitor to connect (for native USB boards)

  Serial.println("I2C Scanner");

  // Initialize the I2C bus
  Wire.begin(); // Defaults to pins SDA=21 and SCL=22 on ESP32

  Serial.println("Scanning for I2C devices...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    // The Wire.endTransmission() function returns:
    // 0: success
    // 1: data too long to fit in buffer
    // 2: received NACK on transmit of address
    // 3: received NACK on transmit of data
    // 4: other error
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("Scan complete");

  delay(5000); // Wait 5 seconds before next scan
}
