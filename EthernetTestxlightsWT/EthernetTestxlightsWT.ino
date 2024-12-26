#include <ETH.h>          //For WT32-ETH01 and ESP-NOW Comms
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define RESET_PIN 32  // GPIO32 connected to ESP EN
#define SIMPLE_LIGHT_NUM 8
uint8_t peerMacAddress[] = {0xD8, 0xBC, 0x38, 0xF8, 0xAF, 0x10};



// Global variables
String type_ = "EthLightHub";
String ver = "2.0";

// Global variables
char device_id[9] {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};

WiFiClient ethClient;
PubSubClient client(ethClient);

// ESP-NOW structure to send data
typedef struct struct_message {
    char topic[50];
    char message[50];;
} struct_message;

struct_message outgoingData;


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

    // ESP-NOW
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(32);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    
    // Print WiFi Channel
    uint8_t channel;
    esp_wifi_get_channel(&channel, NULL);
    client.publish("debug", ("Current WiFi Channel: " + String(channel)).c_str());

    if (esp_now_init() != ESP_OK) {
        client.publish("debug", "Error initializing ESP-NOW");
        return;
    }

    client.publish("debug", "ESP-NOW initialized");
    delay(1000); // Give some time for stabilization

    uint32_t version;
    if (esp_now_get_version(&version) == ESP_OK) {
        client.publish("debug", ("ESP-NOW Version: " + String(version, HEX)).c_str());
    } else {
        client.publish("debug", "Failed to get ESP-NOW version");
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, peerMacAddress, 6);
    peerInfo.channel = 1;  
    peerInfo.encrypt = false;

    // Print Peer Info
    String peerMacStr = "";
    for (int i = 0; i < 6; i++) {
        peerMacStr += String(peerInfo.peer_addr[i], HEX);
        if (i < 5) peerMacStr += ":";
    }
    client.publish("debug", ("Peer Info: MAC: " + peerMacStr).c_str());
    client.publish("debug", ("Channel: " + String(peerInfo.channel)).c_str());
    client.publish("debug", ("Encryption: " + String(peerInfo.encrypt ? "Enabled" : "Disabled")).c_str());

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        esp_err_t err = esp_now_add_peer(&peerInfo);
        client.publish("debug", ("Failed to add ESP-NOW peer. Error code: " + String(err)).c_str());
        client.publish("debug", "Failed to add ESP-NOW peer");
        return;
    }
    esp_now_register_send_cb(OnDataSent);

    String topic {};
    for(int i = 1; i <= SIMPLE_LIGHT_NUM; i++) {
      client.subscribe((String(device_id) + "/" + String("onOff") + String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    for(int i = SIMPLE_LIGHT_NUM+1; i <= 12; i+=2) {
      client.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    client.publish("reporting", "I have subscribed to the topics");
    client.publish("debug", "I have subscribed to the topics");
}


void setup2() {
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
        Serial.println("MQTT connected");
        client.publish("debug", "MQTT connected");
    } else {
        reconnect();
        Serial.println("MQTT connection failed");
    }

    // ESP-NOW
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(32);
    if (esp_now_init() != ESP_OK) {
      client.publish("debug", "Error initializing ESP-NOW");
      return;
    }
    delay(1000);
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, peerMacAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      client.publish("debug", "Failed to add ESP-NOW peer");
      return;
    }
    esp_now_register_send_cb(OnDataSent);



    String topic {};
    for(int i = 1; i <= SIMPLE_LIGHT_NUM; i++) {
      client.subscribe((String(device_id) + "/" + String("onOff") + String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    for(int i = SIMPLE_LIGHT_NUM+1; i <= 12; i+=2) {
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
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    client.publish("debug", (String("Message arrived on topic: ")+String(topic)).c_str());
    client.publish("debug", (String(" Message: ") + message).c_str());

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
        else {
          // Copy topic and message into the structure
          strncpy(outgoingData.topic, topic, sizeof(outgoingData.topic) - 1);
          outgoingData.topic[sizeof(outgoingData.topic) - 1] = '\0'; // Ensure null-termination
          message.toCharArray(outgoingData.message, sizeof(outgoingData.message));
          esp_err_t result = esp_now_send(peerMacAddress, (uint8_t *) &outgoingData, sizeof(outgoingData));
          if (result == ESP_OK) {
            client.publish("debug", "Sent with success");
          } else {
            client.publish("debug", "Error sending the data");
          }
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

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  client.publish("debug", "\r\nLast Packet Send Status:\t");
  client.publish("debug", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}
