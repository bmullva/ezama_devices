#include <ETH.h>          //For WT32-ETH01
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <ModbusRTU.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define MODBUS_SERIAL Serial2  // Assuming Serial2 for Modbus communication
#define MODBUS_RX 17
#define MODBUS_TX 5
#define RESET_PIN 32  // GPIO32 connected to ESP EN

#define SLAVE_ADDR_1 1
#define SLAVE_ADDR_2 2
#define SLAVE_ADDR_3 3
#define SLAVE_ADDR_4 4
#define SLAVE_ADDR_5 5
#define MAX_STRNG_LEN 32

// Global variables
String type_ = "EthSwitchHub";
String ver = "2.0";

char device_id[9] {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};

WiFiClient ethClient;
PubSubClient client(ethClient);
ModbusRTU mb;

void setup() {
    Serial.begin(115200);
    delay(1000);

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    setupEthernet();
    
    // Initialize Modbus on custom pins
    MODBUS_SERIAL.begin(9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX);
    mb.begin(&MODBUS_SERIAL);
    mb.setBaudrate(9600);

    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.setCallback(callback);
        Serial.println("MQTT connected");
        client.publish("debug", "MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Instead of polling, we'll listen for incoming data
  if (receive()) { // Check if there's any incoming Modbus message
    //uint8_t slaveId = mb._slaveId();
    //uint8_t functionCode = mb._functionCode();
    uint8_t slaveId = mb.getSlaveId();
    uint8_t functionCode = mb.getFunctionCode();
    
    if (functionCode == 0x10) { // Assuming Write Multiple Registers for unsolicited data from slaves
      uint16_t data[1];
      data[0] = (mb._buffer[3] << 8) | mb._buffer[4];
      //uint16_t data[mb.getQuantity()];
      //mb.readHreg(0, data, mb.getQuantity()); // Reading from holding registers
      
      //for (int i = 0; i < mb.getQuantity(); i++) {
        int switchNumber = (data[i] >> 8) & 0xFF;
        int switchState = data[i] & 0xFF;
        String state;
        client.publish("debug",("Received from slave "+String(slaveId)+": Switch "+String(switchNumber)+" State byte "+String(switchState,HEX)).c_str());
        
        // Convert switch state to string
        switch(switchState) {
        case 0x00: state = "on"; break;
        case 0x01: state = "off"; break;
        case 0x02: state = "dim"; break;
        case 0x03: state = "brighten"; break;
        case 0x04: state = "release"; break;
        case 0x05: state = "heat"; break;
        case 0x06: state = "cool"; break;
        case 0x07: state = "dbon"; break;
        case 0x08: state = "dboff"; break;
        default: state = "?"; // Handle unexpected values
      }
        publish_controls(String(switchNumber), state);
      //}
    }
    mb.clearRxBuffer(); // Clear the buffer for the next message
  }
  
  delay(100); // Add delay at the end of loop to not overload CPU
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(device_id)) {
            Serial.println("connected");
            client.subscribe("broadcast");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

bool connectToMQTT() {
    for (int i = 0; i < sizeof(mqtt_ips) / sizeof(mqtt_ips[0]); i++) {
        client.setServer(mqtt_ips[i], 1883);
        if (client.connect(device_id)) {
            return true;
        }
    }
    return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(" Message: ");
    Serial.println(message);

    if (strcmp(topic, "broadcast") == 0) {
        if (message == "ping") {
          publish_reporting_json();
        }
        else if (message == "restart") {
            delay(10000);
            ESP.restart();
        }
    } else if (strcmp(topic, device_id) == 0) {
        if (message == "restart") ESP.restart();
        else if (message == "reset") {
            for (int i = 0; i < 222; i++) EEPROM.write(i, 255);
            for (int i = 231; i < 512; i++) EEPROM.write(i, 255);
            EEPROM.write(230, 's');
            EEPROM.commit();
            ESP.restart();
        }
    } 
}

void setupEthernet() {
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    Serial.println("Ethernet initialized");

    while (!ETH.linkUp()) {
        Serial.println("Waiting for Ethernet link...");
        delay(1000);
    }
    Serial.println("Ethernet link established");
}

void publish_controls2(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  client.publish(topic.c_str(), pin_msg.c_str());
}

void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  client.publish("debug", ("Publishing to topic " + topic + " with message " + pin_msg).c_str());
  client.publish(topic.c_str(), pin_msg.c_str());
}

void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,48,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}

bool receive() {
  bool dataReceived = false;
  uint8_t buffer[256]; // Buffer for incoming data, adjust size as needed
  uint8_t bufferIndex = 0;

  // Check if there's data available to read
  if (MODBUS_SERIAL.available() > 0) {
    // Read data until we get a full Modbus message or timeout
    unsigned long startMillis = millis();
    while (millis() - startMillis < 100) { // Timeout after 100ms if no more data comes
      if (MODBUS_SERIAL.available() > 0) {
        buffer[bufferIndex++] = MODBUS_SERIAL.read();
        // Check for the end of a Modbus frame (3.5 character times for RTU)
        if (millis() - startMillis > 3.5 * (1000.0 / 9600.0 * 11)) { // 3.5 chars at 9600bps, 11 bits per char (8 data + 1 start + 2 stop)
          dataReceived = true;
          break;
        }
      }
    }
    
    if (dataReceived) {
      // Process the received data as Modbus
      //mb.setRxBuffer(buffer, bufferIndex); // Set the received data to Modbus buffer
      //if (mb.checkCRC()) { // Check CRC if your Modbus setup uses it
        memcpy(mb._buffer, buffer, bufferIndex);
        mb._len = bufferIndex; // Assuming _len is public or there's a method to set length
        uint8_t slaveId = buffer[0];
        uint8_t functionCode = buffer[1];
        
        // Store the slave ID and function code for later use
        mb._slaveId = slaveId;
        mb._functionCode = functionCode;
        
        // If you're using registers, you might need to manually parse the data here
        // For simplicity, we assume the data is valid and processed by ModbusRTU methods
        return true;
      //} else {
      //  Serial.println("CRC check failed");
      //}
    }
  }
  return false;
}
