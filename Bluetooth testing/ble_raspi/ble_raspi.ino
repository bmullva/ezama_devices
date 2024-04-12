#include <ArduinoBLE.h>

BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLEWriteWithoutResponse, 20);
BLECharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLENotify, 20);

bool deviceConnected = false;

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  String deviceAddress = BLE.address();
  Serial.print("Device Address (MAC): ");
  Serial.println(deviceAddress);

  // Set the advertised local name (optional)
  BLE.setLocalName("UART Service For ESP32");
  BLE.setAdvertisedService(uartService);

  // Add the characteristics to the service
  uartService.addCharacteristic(rxCharacteristic);
  uartService.addCharacteristic(txCharacteristic);

  // Add service
  BLE.addService(uartService);

  // Start advertising
  BLE.advertise();

  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Your code to send notifications here
  }
}

void BLEDeviceConnected(BLEDevice central) {
  deviceConnected = true;
  Serial.print("Connected to central: ");
  Serial.println(central.address());
}

void BLEDeviceDisconnected(BLEDevice central) {
  deviceConnected = false;
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}

void onReceive(BLEDevice central, BLECharacteristic characteristic) {
  // Your code to handle received data here
}
