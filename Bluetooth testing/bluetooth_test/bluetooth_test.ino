#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run "make menuconfig"
#endif

BluetoothSerial SerialBT; // Create an instance of the BluetoothSerial class

const int GPIO22 = 22; // Define GPIO pin numbers
const int GPIO23 = 23;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Control"); // Initialize Bluetooth with a device name

  pinMode(GPIO22, OUTPUT); // Set GPIO pins as OUTPUT
  pinMode(GPIO23, OUTPUT);

  digitalWrite(GPIO22, LOW); // Initialize pins as LOW
  digitalWrite(GPIO23, LOW);

  Serial.println("Bluetooth started, waiting for connections...");
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read(); // Read the incoming Bluetooth data

    // Process the command received from Bluetooth
    if (command == '1') {
      digitalWrite(GPIO22, HIGH); // Turn on GPIO22
      Serial.println("GPIO22 is ON");
    } else if (command == '0') {
      digitalWrite(GPIO22, LOW); // Turn off GPIO22
      Serial.println("GPIO22 is OFF");
    } else if (command == '3') {
      digitalWrite(GPIO23, HIGH); // Turn on GPIO23
      Serial.println("GPIO23 is ON");
    } else if (command == '2') {
      digitalWrite(GPIO23, LOW); // Turn off GPIO23
      Serial.println("GPIO23 is OFF");
    }
  }
}
