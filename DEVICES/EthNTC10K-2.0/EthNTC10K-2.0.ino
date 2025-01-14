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


// Global variables
String type_ = "EthNTC10K";
String ver = "2.0";

char device_id[9] {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};

WiFiClient ethClient;
PubSubClient mqttClient(ethClient);

const double dVCC = 3.3;             // NodeMCU on board 3.3v vcc
const double dR2 = 10000;            // 10k ohm series resistor
const double dAdcResolution = 4095;  // 12-bit adc

const double dA = 0.001129148;       // thermistor equation parameters
const double dB = 0.000234125;
const double dC = 0.0000000876741; 

double dVout, dRth, dTemperature, dAdcValue;
int setpoint {};


void setup() {
    Serial.begin(115200);
    
    // Initialize Ethernet
    setupEthernet();

    // Initialize EEPROM
    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    // Initialize WiFi for ESP-NOW (not for network connection)
    WiFi.mode(WIFI_MODE_STA);
    esp_wifi_set_max_tx_power(32);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    if (connectToMQTT()) {
        mqttClient.subscribe(device_id);
        mqttClient.subscribe("broadcast");
        mqttClient.setCallback(callback);
        Serial.println("MQTT connected");
        mqttClient.publish("debug", "MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }

    mqttClient.subscribe((String(device_id) + String("/set_temp1")).c_str());
    setpoint = EEPROM.read(220);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();
    delay(50);

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
      publish_controls_json("", "on");
    }
    if (dTemperature > setpoint+3) {
      Serial.println("Turned Off");
      publish_controls_json("", "off");
    }
  
    delay(1000);
}

void reconnect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(device_id)) {
            Serial.println("connected");
            mqttClient.subscribe("broadcast");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

bool connectToMQTT() {
    for (int i = 0; i < sizeof(mqtt_ips) / sizeof(mqtt_ips[0]); i++) {
        mqttClient.setServer(mqtt_ips[i], 1883);
        if (mqttClient.connect(device_id)) {
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
    }  else {
    receive_controls_json(topic, message);
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

void receive_controls_json(String topic, String msg) {
  if (topic == String(device_id) + "/" + String("set_temp1")) {
    setpoint = msg.toInt();
    if (setpoint != EEPROM.read(220)) {
      EEPROM.write(220, setpoint);
      EEPROM.commit();
    }
  }
}

void publish_controls_json(String pin_name, String pin_msg) {
  String topic = String(device_id) + "/1";
  mqttClient.publish(topic.c_str(), pin_msg.c_str());
}

void publish_reporting_json() {
  DynamicJsonDocument state_json(1024);
  String output;
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  state_json["vG"]        = "setpoint,60,100;tempF,40,120";
  state_json["vL"]        = "1,1,set_temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]        = "1,1,";
  serializeJson(state_json, output);
  mqttClient.publish("reporting", output.c_str());

  String topic = String(device_id)+"/tempF";  
  mqttClient.publish(topic.c_str(), String(dTemperature).c_str());

  topic = String(device_id)+"/setpoint";  
  mqttClient.publish(topic.c_str(), String(setpoint).c_str());
}
