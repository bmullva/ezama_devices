#include <ETH.h>           // For WT32-ETH01
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define RESET_PIN 32  // GPIO32 connected to ESP EN
#define SIMPLE_LIGHT_NUM 8
#define UART_TX_PIN 5  // GPIO5 for clock (TX)
#define UART_RX_PIN 17 // GPIO17 for data (RX)

// Global variables
String type_ = "EthLightHub";
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

// Structure for UART communication
struct struct_message {
    char topic[50];
    char message[50];
} outgoingData;

HardwareSerial UART_custom(2); // Use UART2 for communication

void setup() {
    Serial.begin(115200);

    setupEthernet();

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.subscribe("password");
        client.setCallback(callback);
        client.publish("debug", "MQTT connected");
    } else {
        reconnect();
        client.publish("debug", "MQTT connection failed");
    }

    UART_custom.begin(115200, SERIAL_8N1, UART_TX_PIN, UART_RX_PIN); // Initialize UART
    client.publish("debug", "UART initialized");

    for(int i = 1; i <= SIMPLE_LIGHT_NUM; i++) {
      client.subscribe((String(device_id) + "/" + String("onOff") + String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    for(int i = SIMPLE_LIGHT_NUM+1; i <= 12; i+=2) {
      client.subscribe((String(device_id) + "/" + String("onOff") + String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    client.publish("reporting", "I have subscribed to the topics");
    client.publish("debug", "I have subscribed to the topics");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();    
    delay(50); 
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
        }  else {
            Serial.print("Failed to connect to broker at ");
            Serial.print(mqtt_ips[i]);
            Serial.print(", error code = ");
            Serial.println(client.state());
        }
    }
    return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
    String topic_str = String(topic); // Convert char* to String at the beginning
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (topic_str == "broadcast") {
        if (message == "ping") {
          publish_reporting_json();
        }
        else if (message == "restart") {
            delay(10000);
            ESP.restart();
        }
    } else if (topic_str == device_id) {
        if (message == "restart") ESP.restart();
        else if (message == "reset") {
            for (int i = 0; i < 222; i++) EEPROM.write(i, 255);
            for (int i = 231; i < 512; i++) EEPROM.write(i, 255);
            EEPROM.write(230, 's');
            EEPROM.commit();
            ESP.restart();
        }
    } else {
          client.publish("debug", ("Message arrived on topic: " + topic_str).c_str());
          client.publish("debug", (" Message: " + message).c_str());
          
          // Since we're using String, length checking and copying are simplified
          if (topic_str.length() >= sizeof(outgoingData.topic)) {
              client.publish("debug", "Warning: Topic truncated");
          }
          
          // Copy topic and message into the structure
          topic_str.toCharArray(outgoingData.topic, sizeof(outgoingData.topic));
          message.toCharArray(outgoingData.message, sizeof(outgoingData.message));

          // Debug: Publish the UART message to MQTT
          String debugMessage = "UART Send: Topic: " + topic_str + " Message: " + message;
          client.publish("debug", debugMessage.c_str());

          // Send the data over UART
          UART_custom.write((uint8_t*)&outgoingData, sizeof(outgoingData));
          client.publish("debug", "Data sent via UART");
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

void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  state_json["MAC"] = WiFi.macAddress();
  state_json["pS"]= "0,1,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}
