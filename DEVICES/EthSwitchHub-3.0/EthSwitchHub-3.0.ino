#include <ETH.h>          //For WT32-ETH01 and ESP-NOW Comms
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <CD74HC4067.h>


// Constants
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

const int sharedControlPins[4] = {17,5,33,32}; // pins for all six MUXes
const int selectMuxPins[4] = {12,14,15,-1}; // pins for the 7th MUX (selecting between the six MUXes)
const int commonSIGPin = 4; // Analog input pin; could be any available analog pin

CD74HC4067 sharedMux(sharedControlPins[0], sharedControlPins[1], sharedControlPins[2], sharedControlPins[3]);
CD74HC4067 selectMux(selectMuxPins[0], selectMuxPins[1], selectMuxPins[2], selectMuxPins[3]);

String state {};
enum ButtonState {
  OFF,
  ON
};
int d_pin_reading[96];
int d_pin_n1_reading[96];
unsigned long startMillis[96];
int clk[96];
int rel[96];
ButtonState timer[96];

//int d_pin_reading [20]         = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//int d_pin_n1_reading [20]      = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//unsigned long startMillis [20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int clk [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int rel [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//enum ButtonState {
//  OFF,
//  ON
//};
//ButtonState timer[20]          = {OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF};


// Global variables
String type_ = "EthSwitchHub";
String ver = "3.0";

char device_id[9] {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
IPAddress mqtt_ips[] = {IPAddress(192, 168, 0, 222), 
                        IPAddress(192, 168, 1, 222), 
                        IPAddress(192, 168, 4, 222)};

WiFiClient ethClient;
PubSubClient client(ethClient);

void setup() {
    Serial.begin(115200);
    pinMode(commonSIGPin, INPUT_PULLUP);
    for(int i=0; i<96; i++) {
      d_pin_reading[i]        = HIGH;
      d_pin_n1_reading[i]     = HIGH;
      startMillis[i]          = 0;
      clk[i]                  = 0;
      rel[i]                  = 0;
      timer[i]                = OFF;
    }
    // Initialize Ethernet
    setupEthernet();

    // Initialize EEPROM
    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.setCallback(callback);
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
    
    for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
      selectMux.channel(i/16);
      sharedMux.channel(i%16);
      delay(1);
      d_pin_reading[i] = digitalRead(commonSIGPin);
      if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
            state = "release";
            publish_controls(String(i/2), state);
        }        
        // Check for a button press (transition from HIGH to LOW) and start a timer
        if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == OFF) {
            startMillis[i] = millis();
            timer[i] = ON;
        }
        // Check for a button release (transition from LOW to HIGH)
        if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == ON) {
            rel[i] += 1;
        }
        // Check for a button press (transition from HIGH to LOW)
        if (d_pin_n1_reading[i] == HIGH && d_pin_reading[i] == LOW && timer[i] == ON) {
            clk[i] += 1;
        }
        // Check for timer expiration
        if (millis() - startMillis[i] > 500) {
            if (timer[i] == ON) {
                if (clk[i] == 1 && rel[i] == 0) {                     //hold
                    state = db(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 1 && rel[i] == 1) {              //click
                    state = fo(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
                    state = ch(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
                    state = dbl(i);   /* appropriate state function like fo, db, ch, dbl */
                } 
                publish_controls(String(i/2), state);
            }
            // Reset variables and timer
            clk[i] = 0;
            rel[i] = 0;
            timer[i] = OFF;
        }
    
        // Update the previous reading for the next iteration
        d_pin_n1_reading[i] = d_pin_reading[i];
    }
    
  delay(50);
}

String db(int x) {
  if (x%2 == 0) { 
    return "dim";
  }
  if (x%2 != 0) {
    return "brighten";
  }
}

String fo(int x) {
  if (x%2 == 0) { 
    return "off";
  }
  if (x%2 != 0) {
    return "on";
  }
}

String ch(int x) {
  if (x%2 == 0) { 
    return "cool";
  }
  if (x%2 != 0) {
    return "heat";
  }
}

String dbl(int x) {
  if (x%2 == 0) { 
    return "dboff";
  }
  if (x%2 != 0) {
    return "dbon";
  }
}

void deleteMe() {
    for(int mux = 0; mux < 6; mux++) {
    selectMux.channel(mux);
    delay(1);
    
    for(int channel = 0; channel < 16; channel++) {
      sharedMux.channel(channel);
      delay(1);
      
      int sensorValue = digitalRead(commonSIGPin);  // Changed to digitalRead
      
      if (sensorValue == LOW) {
        // Prepare the message
        char message[50];
        snprintf(message, sizeof(message), "MUX %d, Channel %d is LOW", mux, channel);
        
        // Publish to MQTT topic
        client.publish("debug", message);
      }
    }
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
  state_json["pS"]= "0,47,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}
