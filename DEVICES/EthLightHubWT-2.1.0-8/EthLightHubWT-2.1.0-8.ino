#include <ETH.h>           // For WT32-ETH01
#include <WiFi.h>          // Needed for ESP-NOW
#include <esp_wifi.h>
#include <esp_now.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define RESET_PIN 32  // GPIO32 connected to ESP EN
#define SIMPLE_LIGHT_NUM 8

// Global variables
String type_ = "EthLightHub";
String ver = "2.1.0-8";

uint8_t macAddressSlave[] = {0xC8, 0x2E, 0x18, 0xC0, 0xA2, 0xD4};
char device_id[9] {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};

WiFiClient ethClient;
PubSubClient client(ethClient);

// Structure for ESP-NOW communication
struct struct_message {
    char topic[50];
    char message[50];
};

// ESP-NOW Callback function
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);

    setupEthernet();
    WiFi.mode(WIFI_STA);  // Set device as a Wi-Fi Station
    esp_wifi_set_max_tx_power(32);
    
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, macAddressSlave, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

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
          
        struct_message outgoingData;
        topic_str.toCharArray(outgoingData.topic, sizeof(outgoingData.topic));
        message.toCharArray(outgoingData.message, sizeof(outgoingData.message));

        // Send via ESP-NOW
        esp_err_t result = esp_now_send(macAddressSlave, (uint8_t *) &outgoingData, sizeof(outgoingData));
        if (result == ESP_OK) {
            Serial.println("Sent with success");
        } else {
            Serial.println("Error sending the data");
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

void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  state_json["MAC"] = WiFi.macAddress();
  //state_json["vG"]        = "amp,0,20";
  state_json["vL"]        = "1,12,onOff;1,12,lux;9,11,temp";
  state_json["pL"]        = "1,11,";
  //state_json["pS"]= "0,47,";


  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}
