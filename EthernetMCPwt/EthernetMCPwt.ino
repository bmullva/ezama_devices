#include <ETH.h>          //For WT32-ETH01 - TEST 3 basis
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>
//#include <Wire.h>

// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

#define SDA 17
#define SCL 5
#define RESET_PIN 32  // GPIO32 connected to ESP EN
//#define MCP_ADDR_1 0x21
//#define MCP_ADDR_2 0x22
//#define MCP_ADDR_3 0x23
#define MCP_ADDR_4 0x24
//#define MCP_ADDR_5 0x25
//#define MCP_ADDR_6 0x26
//#define MCP_ADDR_7 0x27


// Global variables
String type_ = "EthSwitchHub";
String ver = "2.0";

char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};
char device_id[9] {};
int d_pin_reading [16]         = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
int d_pin_n1_reading [16]      = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
unsigned long startMillis [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int clk [16]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int rel [16]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int mom_pins [16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
enum ButtonState {
  OFF,
  ON
};
ButtonState timer[16]          = {OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF};

WiFiClient ethClient;
PubSubClient client(ethClient);
Adafruit_MCP23X17 mcp4;

void setup() {
    Serial.begin(115200);
    delay(1000);
    resetMCP23017();
    delay(1000);
    mcp4.begin_I2C(MCP_ADDR_4);
    
    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    setupEthernet();
    
    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.setCallback(callback);
        Serial.println("MQTT connected");
        client.publish("debug", "MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }

    for(int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
      pinMode(mom_pins[i], INPUT_PULLUP);;
    }
  
}

void loop() {
    if (!client.connected()) {
        unsigned long currentMillis = millis();
        static unsigned long previousReconnectAttempt = 0;
        const unsigned long reconnectInterval = 5000; // 5 seconds

        if (currentMillis - previousReconnectAttempt >= reconnectInterval) {
            reconnect();
            previousReconnectAttempt = currentMillis;
        }
    }
    
    client.loop();

  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(mom_pins[i]);
  }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 16; i++) {  
    
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
      Serial.println("Detected release on pin");
      publish_controls(String(sw(i)), String("release"));          // release
    }
    
    // Check for a button press (transition from HIGH to LOW) and start a timer
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == OFF) {
      Serial.println("Button press on pin");
      startMillis[i] = millis();
      timer[i] = ON;
    }
  
    // Check for a button release (transition from LOW to HIGH)
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == ON) {
      Serial.println("Button release on pin ");
      rel[i] += 1;
    }

    // Check for a button press (transition from HIGH to LOW)
    if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == ON) {
      Serial.println("Additional press on pin");
      clk[i] += 1;
    }
    
    // Check for timer expiration
    if (millis() - startMillis[i] > 500) {
      if (timer[i] == ON) {
        if (clk[i] == 1 && rel[i] == 0) {                     //hold
          publish_controls(String(sw(i)), db(sw(i)));
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          publish_controls(String(sw(i)), fo(sw(i))); 
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          publish_controls(String(sw(i)), ch(sw(i)));
        } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
          publish_controls(String(sw(i)), dbl(sw(i)));
        } 
      }
      // Reset variables and timer
      clk[i] = 0;
      rel[i] = 0;
      timer[i] = OFF;
    }
  
    // Update the previous reading for the next iteration
    d_pin_n1_reading[i] = d_pin_reading[i];
  }

  delay(100);
}

int sw(int x) {
  return x/2;
}

String db(int x) {
  if (x%2 == 0) { 
    return String("dim");
  }
  if (x%2 != 0) {
    return String("brighten");
  }
}

String fo(int x) {
  if (x%2 == 0) { 
    return String("off");
  }
  if (x%2 != 0) {
    return String("on");
  }
}

String ch(int x) {
  if (x%2 == 0) { 
    return String("cool");
  }
  if (x%2 != 0) {
    return String("heat");
  }
}

String dbl(int x) {
  if (x%2 == 0) { 
    return String("double-off");
  }
  if (x%2 != 0) {
    return String("double-on");
  }
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

void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
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
  state_json["pS"]= "0,16,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}

void resetMCP23017() {
    digitalWrite(RESET_PIN, LOW); 
    delay(10);                     
    digitalWrite(RESET_PIN, HIGH);
}
