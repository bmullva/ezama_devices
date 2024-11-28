#include <ETH.h>          //For ESP32 Dev Module
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

/*
const int GPA0 = 0;   // GPIOA pin 0
const int GPA1 = 1;   // GPIOA pin 1
const int GPA2 = 2;   // GPIOA pin 2
const int GPA3 = 3;   // GPIOA pin 3
const int GPA4 = 4;   // GPIOA pin 4
const int GPA5 = 5;   // GPIOA pin 5
const int GPA6 = 6;   // GPIOA pin 6
const int GPA7 = 7;   // GPIOA pin 7

const int GPB0 = 8;   // GPIOB pin 0
const int GPB1 = 9;   // GPIOB pin 1
const int GPB2 = 10;  // GPIOB pin 2
const int GPB3 = 11;  // GPIOB pin 3
const int GPB4 = 12;  // GPIOB pin 4
const int GPB5 = 13;  // GPIOB pin 5
const int GPB6 = 14;  // GPIOB pin 6
const int GPB7 = 15;  // GPIOB pin 7

A2 A1 A0 
000 = 0x20
001 = 0x21
010 = 0x22
011 = 0x23
100 = 0x24
101 = 0x25
110 = 0x26
111 = 0x27
VSS = GND
VDD = 5V
*/


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Ethernet Switch Hub";
String ver = "2.0";

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   -1
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define LED_PIN         2

WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
Adafruit_MCP23X17 mcp0;
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;
Adafruit_MCP23X17 mcp3;
Adafruit_MCP23X17 mcp4;
Adafruit_MCP23X17 mcp5;
Adafruit_MCP23X17 mcp6;
Adafruit_MCP23X17 mcp7;

//const char* mqtt_server = "192.168.4.222";
const char* mqtt_server {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
char device_id[9] {};
const int mqtt_port = 1883;
const char* mqtt_topic_subscribe = "broadcast";
const char* mqtt_topic_publish = "reporting";
static bool eth_connected = false;
//const int device_id_addr = 222; 
//8 digit (222-229) device_id
const int password_length_addr = 231; // 8 <= len <= 63
// 232-239 NOT USED
const int password_addr = 240; // 8-63 byte (240-302)

int mcp0_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp1_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp2_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp3_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp4_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp5_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp6_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp7_reading [12]         = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

int mcp0_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp1_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp2_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp3_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp4_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp5_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp6_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int mcp7_n1_reading [12]      = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

unsigned long startMillis0 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis1 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis2 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis3 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis4 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis5 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis6 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long startMillis7 [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int clk0 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk1 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk2 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk3 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk4 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk5 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk6 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk7 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int rel0 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel1 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel2 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel3 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel4 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel5 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel6 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel7 [12]                   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int mom_pins [12]              = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13};

// Define enums for different states
enum ButtonState {OFF, ON};

ButtonState timer0[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer1[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer2[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer3[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer4[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer5[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer6[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};
ButtonState timer7[12]         = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

//Need to install the following:
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      mqttClient.setServer(mqtt_server, mqtt_port);
      mqttClient.setCallback(mqttCallback);
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println(messageTemp);
  if (strcmp(topic, "broadcast") == 0) {
    if (messageTemp == "ping") {publish_reporting_json();}
    if (messageTemp == "restart") { 
      delay(10000);  // Delay before restarting
      ESP.restart();
    }
  }
  else if (strcmp(topic, device_id) == 0) {
    if (messageTemp == "restart") {ESP.restart();}
    if (messageTemp == "reset") {
      for (int i = 0; i < 222; i++) {EEPROM.write(i, 255);}
      for (int i = 231; i < 512; i++) {EEPROM.write(i, 255);}
      EEPROM.write(230, 's');
      EEPROM.commit();
      ESP.restart();
    }
  }
  else if (strcmp(topic, "password") == 0) {
    int password_length = messageTemp.length();
    if (password_length > 0 && password_length <= 64) {  // Ensure password is within bounds
      EEPROM.write(password_length_addr, password_length);
      for (int i = 0; i < password_length; i++) {
        EEPROM.write(password_addr + i, messageTemp[i]);
      }
      EEPROM.commit();
      ESP.restart();
    }
    else {
      Serial.println("Invalid password length.");
    }
  }
  else {
    receive_controls_json(topic, messageTemp);
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type_;
  state_json["ver"] = ver;
  state_json["IP"] = ETH.localIP();
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,23,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);
}

// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
}

// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
// Only these will be published: "on", "off", "dim", "brighten", "heat", "cool"

void publish_controls(String switch_num, String pin_msg) {
  String topic = String(device_id) + "/" + switch_num;
  mqttClient.publish(topic.c_str(), pin_msg.c_str());
}


//6 SETUP (pins)
void specific_connect() {
}

void findMQTTserver() {
  mqttClient.setServer(mqtt_ip_1, 1883);
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_1;
    return;  // Connected successfully
  }
  mqttClient.setServer(mqtt_ip_2, 1883);
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_2;
    return;  // Connected successfully
  }
  mqttClient.setServer(mqtt_ip_3, 1883);
  if (mqttClient.connect(device_id)) {
    mqtt_server = mqtt_ip_3;
    return;  // Connected successfully
  }
}

void setup() {
  delay(1000);
  if (!mcp0.begin_I2C(0x20)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp1.begin_I2C(0x21)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp2.begin_I2C(0x22)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp3.begin_I2C(0x23)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp4.begin_I2C(0x24)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp5.begin_I2C(0x25)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp6.begin_I2C(0x26)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  if (!mcp7.begin_I2C(0x27)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
  
  mcp0.pinMode(0, INPUT_PULLUP);
  mcp0.pinMode(1, INPUT_PULLUP);
  mcp0.pinMode(2, INPUT_PULLUP);
  mcp0.pinMode(3, INPUT_PULLUP);
  mcp0.pinMode(4, INPUT_PULLUP);
  mcp0.pinMode(5, INPUT_PULLUP);
  mcp0.pinMode(8, INPUT_PULLUP);
  mcp0.pinMode(9, INPUT_PULLUP);
  mcp0.pinMode(10, INPUT_PULLUP);
  mcp0.pinMode(11, INPUT_PULLUP);

  mcp1.pinMode(0, INPUT_PULLUP);
  mcp1.pinMode(1, INPUT_PULLUP);
  mcp1.pinMode(2, INPUT_PULLUP);
  mcp1.pinMode(3, INPUT_PULLUP);
  mcp1.pinMode(4, INPUT_PULLUP);
  mcp1.pinMode(5, INPUT_PULLUP);
  mcp1.pinMode(8, INPUT_PULLUP);
  mcp1.pinMode(9, INPUT_PULLUP);
  mcp1.pinMode(10, INPUT_PULLUP);
  mcp1.pinMode(11, INPUT_PULLUP);

  mcp2.pinMode(0, INPUT_PULLUP);
  mcp2.pinMode(1, INPUT_PULLUP);
  mcp2.pinMode(2, INPUT_PULLUP);
  mcp2.pinMode(3, INPUT_PULLUP);
  mcp2.pinMode(4, INPUT_PULLUP);
  mcp2.pinMode(5, INPUT_PULLUP);
  mcp2.pinMode(8, INPUT_PULLUP);
  mcp2.pinMode(9, INPUT_PULLUP);
  mcp2.pinMode(10, INPUT_PULLUP);
  mcp2.pinMode(11, INPUT_PULLUP);

  mcp3.pinMode(0, INPUT_PULLUP);
  mcp3.pinMode(1, INPUT_PULLUP);
  mcp3.pinMode(2, INPUT_PULLUP);
  mcp3.pinMode(3, INPUT_PULLUP);
  mcp3.pinMode(4, INPUT_PULLUP);
  mcp3.pinMode(5, INPUT_PULLUP);
  mcp3.pinMode(8, INPUT_PULLUP);
  mcp3.pinMode(9, INPUT_PULLUP);
  mcp3.pinMode(10, INPUT_PULLUP);
  mcp3.pinMode(11, INPUT_PULLUP);

  mcp4.pinMode(0, INPUT_PULLUP);
  mcp4.pinMode(1, INPUT_PULLUP);
  mcp4.pinMode(2, INPUT_PULLUP);
  mcp4.pinMode(3, INPUT_PULLUP);
  mcp4.pinMode(4, INPUT_PULLUP);
  mcp4.pinMode(5, INPUT_PULLUP);
  mcp4.pinMode(8, INPUT_PULLUP);
  mcp4.pinMode(9, INPUT_PULLUP);
  mcp4.pinMode(10, INPUT_PULLUP);
  mcp4.pinMode(11, INPUT_PULLUP);

  mcp5.pinMode(0, INPUT_PULLUP);
  mcp5.pinMode(1, INPUT_PULLUP);
  mcp5.pinMode(2, INPUT_PULLUP);
  mcp5.pinMode(3, INPUT_PULLUP);
  mcp5.pinMode(4, INPUT_PULLUP);
  mcp5.pinMode(5, INPUT_PULLUP);
  mcp5.pinMode(8, INPUT_PULLUP);
  mcp5.pinMode(9, INPUT_PULLUP);
  mcp5.pinMode(10, INPUT_PULLUP);
  mcp5.pinMode(11, INPUT_PULLUP);

  mcp6.pinMode(0, INPUT_PULLUP);
  mcp6.pinMode(1, INPUT_PULLUP);
  mcp6.pinMode(2, INPUT_PULLUP);
  mcp6.pinMode(3, INPUT_PULLUP);
  mcp6.pinMode(4, INPUT_PULLUP);
  mcp6.pinMode(5, INPUT_PULLUP);
  mcp6.pinMode(8, INPUT_PULLUP);
  mcp6.pinMode(9, INPUT_PULLUP);
  mcp6.pinMode(10, INPUT_PULLUP);
  mcp6.pinMode(11, INPUT_PULLUP);

  mcp7.pinMode(0, INPUT_PULLUP);
  mcp7.pinMode(1, INPUT_PULLUP);
  mcp7.pinMode(2, INPUT_PULLUP);
  mcp7.pinMode(3, INPUT_PULLUP);
  mcp7.pinMode(4, INPUT_PULLUP);
  mcp7.pinMode(5, INPUT_PULLUP);
  mcp7.pinMode(8, INPUT_PULLUP);
  mcp7.pinMode(9, INPUT_PULLUP);
  mcp7.pinMode(10, INPUT_PULLUP);
  mcp7.pinMode(11, INPUT_PULLUP);

  Serial.begin(115200);
  EEPROM.begin(512);
  for (int i = 0; i < 8; i++) {
    device_id[i] = char(EEPROM.read(222 + i));
    }
  Serial.println("Ether Switch Begin\n");
  WiFi.onEvent(WiFiEvent);
  Serial.printf("%d\n\r", ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE));
  findMQTTserver();
  mqttClient.subscribe(mqtt_topic_subscribe);
  //mqttClient.setServer(mqtt_server, mqtt_port);
}


//7 MAIN LOOP#####
// Only these will be published: "click", "on", "off", "dim", "brighten", "heat", "cool"

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  //Serial.println(eth_connected);
  if (eth_connected) {
    // Loop through an array of digital pins, read their states, and store the readings
    //for(int i = 0; i < sizeof(mcp0_reading) / sizeof(mcp0_reading[0]); i++) {
    for(int i = 0; i < 12; i++) {
      mcp0_reading[i] = mcp0.digitalRead(mom_pins[i]);
      mcp1_reading[i] = mcp1.digitalRead(mom_pins[i]);
      mcp2_reading[i] = mcp2.digitalRead(mom_pins[i]);
      mcp3_reading[i] = mcp3.digitalRead(mom_pins[i]);
      mcp4_reading[i] = mcp4.digitalRead(mom_pins[i]);
      mcp5_reading[i] = mcp5.digitalRead(mom_pins[i]);
      mcp6_reading[i] = mcp6.digitalRead(mom_pins[i]);
      mcp7_reading[i] = mcp7.digitalRead(mom_pins[i]);
    }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 12; i++) {  
    
    if (mcp0_n1_reading[i] == LOW && mcp0_reading[i] == HIGH && timer0[i] == OFF) {
          if (i == 0) {publish_controls("0/0", "release");}
          if (i == 1) {publish_controls("0/0", "release");}
          if (i == 2) {publish_controls("0/1", "release");}
          if (i == 3) {publish_controls("0/1", "release");}
          if (i == 4) {publish_controls("1/0", "release");}
          if (i == 5) {publish_controls("1/0", "release");}
          if (i == 6) {publish_controls("1/1", "release");}
          if (i == 7) {publish_controls("1/1", "release");}
          if (i == 8) {publish_controls("2/0", "release");}
          if (i == 9) {publish_controls("2/0", "release");}
          if (i ==10) {publish_controls("2/1", "release");}
          if (i ==11) {publish_controls("2/1", "release");}
    }

    if (mcp1_n1_reading[i] == LOW && mcp1_reading[i] == HIGH && timer1[i] == OFF) {
          if (i == 0) {publish_controls("3/0", "release");}
          if (i == 1) {publish_controls("3/0", "release");}
          if (i == 2) {publish_controls("3/1", "release");}
          if (i == 3) {publish_controls("3/1", "release");}
          if (i == 4) {publish_controls("4/0", "release");}
          if (i == 5) {publish_controls("4/0", "release");}
          if (i == 6) {publish_controls("4/1", "release");}
          if (i == 7) {publish_controls("4/1", "release");}
          if (i == 8) {publish_controls("5/0", "release");}
          if (i == 9) {publish_controls("5/0", "release");}
          if (i ==10) {publish_controls("5/1", "release");}
          if (i ==11) {publish_controls("5/1", "release");}
    }

    if (mcp2_n1_reading[i] == LOW && mcp2_reading[i] == HIGH && timer2[i] == OFF) {
          if (i == 0) {publish_controls("6/0", "release");}
          if (i == 1) {publish_controls("6/0", "release");}
          if (i == 2) {publish_controls("6/1", "release");}
          if (i == 3) {publish_controls("6/1", "release");}
          if (i == 4) {publish_controls("7/0", "release");}
          if (i == 5) {publish_controls("7/0", "release");}
          if (i == 6) {publish_controls("7/1", "release");}
          if (i == 7) {publish_controls("7/1", "release");}
          if (i == 8) {publish_controls("8/0", "release");}
          if (i == 9) {publish_controls("8/0", "release");}
          if (i ==10) {publish_controls("8/1", "release");}
          if (i ==11) {publish_controls("8/1", "release");}
    }

    if (mcp3_n1_reading[i] == LOW && mcp3_reading[i] == HIGH && timer3[i] == OFF) {
          if (i == 0) {publish_controls("9/0", "release");}
          if (i == 1) {publish_controls("9/0", "release");}
          if (i == 2) {publish_controls("9/1", "release");}
          if (i == 3) {publish_controls("9/1", "release");}
          if (i == 4) {publish_controls("10/0", "release");}
          if (i == 5) {publish_controls("10/0", "release");}
          if (i == 6) {publish_controls("10/1", "release");}
          if (i == 7) {publish_controls("10/1", "release");}
          if (i == 8) {publish_controls("11/0", "release");}
          if (i == 9) {publish_controls("11/0", "release");}
          if (i ==10) {publish_controls("11/1", "release");}
          if (i ==11) {publish_controls("11/1", "release");}
    }

    if (mcp4_n1_reading[i] == LOW && mcp4_reading[i] == HIGH && timer4[i] == OFF) {
          if (i == 0) {publish_controls("12/0", "release");}
          if (i == 1) {publish_controls("12/0", "release");}
          if (i == 2) {publish_controls("12/1", "release");}
          if (i == 3) {publish_controls("12/1", "release");}
          if (i == 4) {publish_controls("13/0", "release");}
          if (i == 5) {publish_controls("13/0", "release");}
          if (i == 6) {publish_controls("13/1", "release");}
          if (i == 7) {publish_controls("13/1", "release");}
          if (i == 8) {publish_controls("14/0", "release");}
          if (i == 9) {publish_controls("14/0", "release");}
          if (i ==10) {publish_controls("14/1", "release");}
          if (i ==11) {publish_controls("14/1", "release");}
    }

    if (mcp5_n1_reading[i] == LOW && mcp5_reading[i] == HIGH && timer5[i] == OFF) {
          if (i == 0) {publish_controls("15/0", "release");}
          if (i == 1) {publish_controls("15/0", "release");}
          if (i == 2) {publish_controls("15/1", "release");}
          if (i == 3) {publish_controls("15/1", "release");}
          if (i == 4) {publish_controls("16/0", "release");}
          if (i == 5) {publish_controls("16/0", "release");}
          if (i == 6) {publish_controls("16/1", "release");}
          if (i == 7) {publish_controls("16/1", "release");}
          if (i == 8) {publish_controls("17/0", "release");}
          if (i == 9) {publish_controls("17/0", "release");}
          if (i ==10) {publish_controls("17/1", "release");}
          if (i ==11) {publish_controls("17/1", "release");}
    }

    if (mcp6_n1_reading[i] == LOW && mcp6_reading[i] == HIGH && timer6[i] == OFF) {
          if (i == 0) {publish_controls("18/0", "release");}
          if (i == 1) {publish_controls("18/0", "release");}
          if (i == 2) {publish_controls("18/1", "release");}
          if (i == 3) {publish_controls("18/1", "release");}
          if (i == 4) {publish_controls("19/0", "release");}
          if (i == 5) {publish_controls("19/0", "release");}
          if (i == 6) {publish_controls("19/1", "release");}
          if (i == 7) {publish_controls("19/1", "release");}
          if (i == 8) {publish_controls("20/0", "release");}
          if (i == 9) {publish_controls("20/0", "release");}
          if (i ==10) {publish_controls("20/1", "release");}
          if (i ==11) {publish_controls("20/1", "release");}
    }

    if (mcp7_n1_reading[i] == LOW && mcp7_reading[i] == HIGH && timer7[i] == OFF) {
          if (i == 0) {publish_controls("21/0", "release");}
          if (i == 1) {publish_controls("21/0", "release");}
          if (i == 2) {publish_controls("21/1", "release");}
          if (i == 3) {publish_controls("21/1", "release");}
          if (i == 4) {publish_controls("22/0", "release");}
          if (i == 5) {publish_controls("22/0", "release");}
          if (i == 6) {publish_controls("22/1", "release");}
          if (i == 7) {publish_controls("22/1", "release");}
          if (i == 8) {publish_controls("23/0", "release");}
          if (i == 9) {publish_controls("23/0", "release");}
          if (i ==10) {publish_controls("23/1", "release");}
          if (i ==11) {publish_controls("23/1", "release");}
    }
    
    // Check for a button press (transition from HIGH to LOW) and start a timer
    if (mcp0_n1_reading[i] == HIGH && mcp0_reading[i] == LOW && timer0[i] == OFF) {
      startMillis0[i] = millis();
      timer0[i] = ON;
    }
    if (mcp1_n1_reading[i] == HIGH && mcp1_reading[i] == LOW && timer1[i] == OFF) {
      startMillis1[i] = millis();
      timer1[i] = ON;
    }
    if (mcp2_n1_reading[i] == HIGH && mcp2_reading[i] == LOW && timer2[i] == OFF) {
      startMillis2[i] = millis();
      timer2[i] = ON;
    }
    if (mcp3_n1_reading[i] == HIGH && mcp3_reading[i] == LOW && timer3[i] == OFF) {
      startMillis3[i] = millis();
      timer3[i] = ON;
    }
    if (mcp4_n1_reading[i] == HIGH && mcp4_reading[i] == LOW && timer4[i] == OFF) {
      startMillis4[i] = millis();
      timer4[i] = ON;
    }
    if (mcp5_n1_reading[i] == HIGH && mcp5_reading[i] == LOW && timer5[i] == OFF) {
      startMillis5[i] = millis();
      timer5[i] = ON;
    }
    if (mcp6_n1_reading[i] == HIGH && mcp6_reading[i] == LOW && timer6[i] == OFF) {
      startMillis6[i] = millis();
      timer6[i] = ON;
    }
    if (mcp7_n1_reading[i] == HIGH && mcp7_reading[i] == LOW && timer7[i] == OFF) {
      startMillis7[i] = millis();
      timer7[i] = ON;
    }
  
    // Check for a button release (transition from LOW to HIGH)
    if (mcp0_n1_reading[i] == LOW && mcp0_reading[i] == HIGH && timer0[i] == ON) {
      rel0[i] += 1;
    }
    if (mcp1_n1_reading[i] == LOW && mcp1_reading[i] == HIGH && timer1[i] == ON) {
      rel1[i] += 1;
    }
    if (mcp2_n1_reading[i] == LOW && mcp2_reading[i] == HIGH && timer2[i] == ON) {
      rel2[i] += 1;
    }
    if (mcp3_n1_reading[i] == LOW && mcp3_reading[i] == HIGH && timer3[i] == ON) {
      rel3[i] += 1;
    }
    if (mcp4_n1_reading[i] == LOW && mcp4_reading[i] == HIGH && timer4[i] == ON) {
      rel4[i] += 1;
    }
    if (mcp5_n1_reading[i] == LOW && mcp5_reading[i] == HIGH && timer5[i] == ON) {
      rel5[i] += 1;
    }
    if (mcp6_n1_reading[i] == LOW && mcp6_reading[i] == HIGH && timer6[i] == ON) {
      rel6[i] += 1;
    }
    if (mcp7_n1_reading[i] == LOW && mcp7_reading[i] == HIGH && timer7[i] == ON) {
      rel7[i] += 1;
    }

    // Check for a button press (transition from HIGH to LOW)
    if (mcp0_n1_reading[i] == HIGH && mcp0_reading[i] == LOW && timer0[i] == ON) {
      clk0[i] += 1;
    }
    if (mcp1_n1_reading[i] == HIGH && mcp1_reading[i] == LOW && timer1[i] == ON) {
      clk1[i] += 1;
    }
    if (mcp2_n1_reading[i] == HIGH && mcp2_reading[i] == LOW && timer2[i] == ON) {
      clk2[i] += 1;
    }
    if (mcp3_n1_reading[i] == HIGH && mcp3_reading[i] == LOW && timer3[i] == ON) {
      clk3[i] += 1;
    }
    if (mcp4_n1_reading[i] == HIGH && mcp4_reading[i] == LOW && timer4[i] == ON) {
      clk4[i] += 1;
    }
    if (mcp5_n1_reading[i] == HIGH && mcp5_reading[i] == LOW && timer5[i] == ON) {
      clk5[i] += 1;
    }
    if (mcp6_n1_reading[i] == HIGH && mcp6_reading[i] == LOW && timer6[i] == ON) {
      clk6[i] += 1;
    }
    if (mcp7_n1_reading[i] == HIGH && mcp7_reading[i] == LOW && timer7[i] == ON) {
      clk7[i] += 1;
    }
    
    // Check for timer expiration
    if (millis() - startMillis0[i] > 500) {
      if (timer0[i] == ON) {
        if (clk0[i] == 1 && rel0[i] == 0) {                     //hold
          if (i == 0) {publish_controls("0/0", "dim");}
          if (i == 1) {publish_controls("0/0", "brighten");}
          if (i == 2) {publish_controls("0/1", "dim");}
          if (i == 3) {publish_controls("0/1", "brighten");}
          if (i == 4) {publish_controls("1/0", "dim");}
          if (i == 5) {publish_controls("1/0", "brighten");}
          if (i == 6) {publish_controls("1/1", "dim");}
          if (i == 7) {publish_controls("1/1", "brighten");}
          if (i == 8) {publish_controls("2/0", "dim");}
          if (i == 9) {publish_controls("2/0", "brighten");}
          if (i == 10) {publish_controls("2/1", "dim");}
          if (i == 11) {publish_controls("2/1", "brighten");}
        } else if (clk0[i] == 1 && rel0[i] == 1) {              //click
          if (i == 0) {publish_controls("0/0", "off");}
          if (i == 1) {publish_controls("0/0", "on");}
          if (i == 2) {publish_controls("0/1", "off");}
          if (i == 3) {publish_controls("0/1", "on");}
          if (i == 4) {publish_controls("1/0", "off");}
          if (i == 5) {publish_controls("1/0", "on");}
          if (i == 6) {publish_controls("1/1", "off");}
          if (i == 7) {publish_controls("1/1", "on");} 
          if (i == 8) {publish_controls("2/0", "off");}
          if (i == 9) {publish_controls("2/0", "on");}
          if (i == 10) {publish_controls("2/1", "off");}
          if (i == 11) {publish_controls("2/1", "on");}   
        } else if (clk0[i] == 2 && rel0[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("0/0", "cool");}
          if (i == 1) {publish_controls("0/0", "heat");}
          if (i == 2) {publish_controls("0/1", "cool");}
          if (i == 3) {publish_controls("0/1", "heat");}
          if (i == 4) {publish_controls("1/0", "cool");}
          if (i == 5) {publish_controls("1/0", "heat");}
          if (i == 6) {publish_controls("1/1", "cool");}
          if (i == 7) {publish_controls("1/1", "heat");}
          if (i == 8) {publish_controls("2/0", "cool");}
          if (i == 9) {publish_controls("2/0", "heat");}
          if (i == 10) {publish_controls("2/1", "cool");}
          if (i == 11) {publish_controls("2/1", "heat");} 
        } //else if (clk0[i] == 2 && rel0[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("0/0", "??");}
         // if (i == 1) {publish_controls("0/0", "??");}
         // if (i == 2) {publish_controls("0/1", "??");}
         // if (i == 3) {publish_controls("0/1", "??");}
         // if (i == 4) {publish_controls("1/0", "??");}
         // if (i == 5) {publish_controls("1/0", "??");}
         // if (i == 6) {publish_controls("1/1", "??");}
         // if (i == 7) {publish_controls("1/1", "??");}
         // if (i == 8) {publish_controls("2/0", "??");}
         // if (i == 9) {publish_controls("2/0", "??");}
         // if (i == 10) {publish_controls("2/1", "??");}
         // if (i == 11) {publish_controls("2/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk0[i] = 0;
      rel0[i] = 0;
      timer0[i] = OFF;
    }

    if (millis() - startMillis1[i] > 500) {
      if (timer1[i] == ON) {
        if (clk1[i] == 1 && rel1[i] == 0) {                     //hold
          if (i == 0) {publish_controls("3/0", "dim");}
          if (i == 1) {publish_controls("3/0", "brighten");}
          if (i == 2) {publish_controls("3/1", "dim");}
          if (i == 3) {publish_controls("3/1", "brighten");}
          if (i == 4) {publish_controls("4/0", "dim");}
          if (i == 5) {publish_controls("4/0", "brighten");}
          if (i == 6) {publish_controls("4/1", "dim");}
          if (i == 7) {publish_controls("4/1", "brighten");}
          if (i == 8) {publish_controls("5/0", "dim");}
          if (i == 9) {publish_controls("5/0", "brighten");}
          if (i == 10) {publish_controls("5/1", "dim");}
          if (i == 11) {publish_controls("5/1", "brighten");}
        } else if (clk1[i] == 1 && rel1[i] == 1) {              //click
          if (i == 0) {publish_controls("3/0", "off");}
          if (i == 1) {publish_controls("3/0", "on");}
          if (i == 2) {publish_controls("3/1", "off");}
          if (i == 3) {publish_controls("3/1", "on");}
          if (i == 4) {publish_controls("4/0", "off");}
          if (i == 5) {publish_controls("4/0", "on");}
          if (i == 6) {publish_controls("4/1", "off");}
          if (i == 7) {publish_controls("4/1", "on");} 
          if (i == 8) {publish_controls("5/0", "off");}
          if (i == 9) {publish_controls("5/0", "on");}
          if (i == 10) {publish_controls("5/1", "off");}
          if (i == 11) {publish_controls("5/1", "on");}   
        } else if (clk1[i] == 2 && rel1[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("3/0", "cool");}
          if (i == 1) {publish_controls("3/0", "heat");}
          if (i == 2) {publish_controls("3/1", "cool");}
          if (i == 3) {publish_controls("3/1", "heat");}
          if (i == 4) {publish_controls("4/0", "cool");}
          if (i == 5) {publish_controls("4/0", "heat");}
          if (i == 6) {publish_controls("4/1", "cool");}
          if (i == 7) {publish_controls("4/1", "heat");}
          if (i == 8) {publish_controls("5/0", "cool");}
          if (i == 9) {publish_controls("5/0", "heat");}
          if (i == 10) {publish_controls("5/1", "cool");}
          if (i == 11) {publish_controls("5/1", "heat");} 
        } //else if (clk1[i] == 2 && rel1[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("3/0", "??");}
         // if (i == 1) {publish_controls("3/0", "??");}
         // if (i == 2) {publish_controls("3/1", "??");}
         // if (i == 3) {publish_controls("3/1", "??");}
         // if (i == 4) {publish_controls("4/0", "??");}
         // if (i == 5) {publish_controls("4/0", "??");}
         // if (i == 6) {publish_controls("4/1", "??");}
         // if (i == 7) {publish_controls("4/1", "??");}
         // if (i == 8) {publish_controls("5/0", "??");}
         // if (i == 9) {publish_controls("5/0", "??");}
         // if (i == 10) {publish_controls("5/1", "??");}
         // if (i == 11) {publish_controls("5/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk1[i] = 0;
      rel1[i] = 0;
      timer1[i] = OFF;
    }

    if (millis() - startMillis2[i] > 500) {
      if (timer2[i] == ON) {
        if (clk2[i] == 1 && rel2[i] == 0) {                     //hold
          if (i == 0) {publish_controls("6/0", "dim");}
          if (i == 1) {publish_controls("6/0", "brighten");}
          if (i == 2) {publish_controls("6/1", "dim");}
          if (i == 3) {publish_controls("6/1", "brighten");}
          if (i == 4) {publish_controls("7/0", "dim");}
          if (i == 5) {publish_controls("7/0", "brighten");}
          if (i == 6) {publish_controls("7/1", "dim");}
          if (i == 7) {publish_controls("7/1", "brighten");}
          if (i == 8) {publish_controls("8/0", "dim");}
          if (i == 9) {publish_controls("8/0", "brighten");}
          if (i == 10) {publish_controls("8/1", "dim");}
          if (i == 11) {publish_controls("8/1", "brighten");}
        } else if (clk2[i] == 1 && rel2[i] == 1) {              //click
          if (i == 0) {publish_controls("6/0", "off");}
          if (i == 1) {publish_controls("6/0", "on");}
          if (i == 2) {publish_controls("6/1", "off");}
          if (i == 3) {publish_controls("6/1", "on");}
          if (i == 4) {publish_controls("7/0", "off");}
          if (i == 5) {publish_controls("7/0", "on");}
          if (i == 6) {publish_controls("7/1", "off");}
          if (i == 7) {publish_controls("7/1", "on");} 
          if (i == 8) {publish_controls("8/0", "off");}
          if (i == 9) {publish_controls("8/0", "on");}
          if (i == 10) {publish_controls("8/1", "off");}
          if (i == 11) {publish_controls("8/1", "on");}   
        } else if (clk2[i] == 2 && rel2[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("6/0", "cool");}
          if (i == 1) {publish_controls("6/0", "heat");}
          if (i == 2) {publish_controls("6/1", "cool");}
          if (i == 3) {publish_controls("6/1", "heat");}
          if (i == 4) {publish_controls("7/0", "cool");}
          if (i == 5) {publish_controls("7/0", "heat");}
          if (i == 6) {publish_controls("7/1", "cool");}
          if (i == 7) {publish_controls("7/1", "heat");}
          if (i == 8) {publish_controls("8/0", "cool");}
          if (i == 9) {publish_controls("8/0", "heat");}
          if (i == 10) {publish_controls("8/1", "cool");}
          if (i == 11) {publish_controls("8/1", "heat");} 
        } //else if (clk2[i] == 2 && rel2[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("6/0", "??");}
         // if (i == 1) {publish_controls("6/0", "??");}
         // if (i == 2) {publish_controls("6/1", "??");}
         // if (i == 3) {publish_controls("6/1", "??");}
         // if (i == 4) {publish_controls("7/0", "??");}
         // if (i == 5) {publish_controls("7/0", "??");}
         // if (i == 6) {publish_controls("7/1", "??");}
         // if (i == 7) {publish_controls("7/1", "??");}
         // if (i == 8) {publish_controls("8/0", "??");}
         // if (i == 9) {publish_controls("8/0", "??");}
         // if (i == 10) {publish_controls("8/1", "??");}
         // if (i == 11) {publish_controls("8/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk2[i] = 0;
      rel2[i] = 0;
      timer2[i] = OFF;
    }

    if (millis() - startMillis3[i] > 500) {
      if (timer3[i] == ON) {
        if (clk3[i] == 1 && rel3[i] == 0) {                     //hold
          if (i == 0) {publish_controls("9/0", "dim");}
          if (i == 1) {publish_controls("9/0", "brighten");}
          if (i == 2) {publish_controls("9/1", "dim");}
          if (i == 3) {publish_controls("9/1", "brighten");}
          if (i == 4) {publish_controls("10/0", "dim");}
          if (i == 5) {publish_controls("10/0", "brighten");}
          if (i == 6) {publish_controls("10/1", "dim");}
          if (i == 7) {publish_controls("10/1", "brighten");}
          if (i == 8) {publish_controls("11/0", "dim");}
          if (i == 9) {publish_controls("11/0", "brighten");}
          if (i == 10) {publish_controls("11/1", "dim");}
          if (i == 11) {publish_controls("11/1", "brighten");}
        } else if (clk3[i] == 1 && rel3[i] == 1) {              //click
          if (i == 0) {publish_controls("9/0", "off");}
          if (i == 1) {publish_controls("9/0", "on");}
          if (i == 2) {publish_controls("9/1", "off");}
          if (i == 3) {publish_controls("9/1", "on");}
          if (i == 4) {publish_controls("10/0", "off");}
          if (i == 5) {publish_controls("10/0", "on");}
          if (i == 6) {publish_controls("10/1", "off");}
          if (i == 7) {publish_controls("10/1", "on");} 
          if (i == 8) {publish_controls("11/0", "off");}
          if (i == 9) {publish_controls("11/0", "on");}
          if (i == 10) {publish_controls("11/1", "off");}
          if (i == 11) {publish_controls("11/1", "on");}   
        } else if (clk3[i] == 2 && rel3[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("9/0", "cool");}
          if (i == 1) {publish_controls("9/0", "heat");}
          if (i == 2) {publish_controls("9/1", "cool");}
          if (i == 3) {publish_controls("9/1", "heat");}
          if (i == 4) {publish_controls("10/0", "cool");}
          if (i == 5) {publish_controls("10/0", "heat");}
          if (i == 6) {publish_controls("10/1", "cool");}
          if (i == 7) {publish_controls("10/1", "heat");}
          if (i == 8) {publish_controls("11/0", "cool");}
          if (i == 9) {publish_controls("11/0", "heat");}
          if (i == 10) {publish_controls("11/1", "cool");}
          if (i == 11) {publish_controls("11/1", "heat");} 
        } //else if (clk3[i] == 2 && rel3[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("9/0", "??");}
         // if (i == 1) {publish_controls("9/0", "??");}
         // if (i == 2) {publish_controls("9/1", "??");}
         // if (i == 3) {publish_controls("9/1", "??");}
         // if (i == 4) {publish_controls("10/0", "??");}
         // if (i == 5) {publish_controls("10/0", "??");}
         // if (i == 6) {publish_controls("10/1", "??");}
         // if (i == 7) {publish_controls("10/1", "??");}
         // if (i == 8) {publish_controls("11/0", "??");}
         // if (i == 9) {publish_controls("11/0", "??");}
         // if (i == 10) {publish_controls("11/1", "??");}
         // if (i == 11) {publish_controls("11/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk3[i] = 0;
      rel3[i] = 0;
      timer3[i] = OFF;
    }
    
    if (millis() - startMillis4[i] > 500) {
      if (timer4[i] == ON) {
        if (clk4[i] == 1 && rel4[i] == 0) {                     //hold
          if (i == 0) {publish_controls("12/0", "dim");}
          if (i == 1) {publish_controls("12/0", "brighten");}
          if (i == 2) {publish_controls("12/1", "dim");}
          if (i == 3) {publish_controls("12/1", "brighten");}
          if (i == 4) {publish_controls("13/0", "dim");}
          if (i == 5) {publish_controls("13/0", "brighten");}
          if (i == 6) {publish_controls("13/1", "dim");}
          if (i == 7) {publish_controls("13/1", "brighten");}
          if (i == 8) {publish_controls("14/0", "dim");}
          if (i == 9) {publish_controls("14/0", "brighten");}
          if (i == 10) {publish_controls("14/1", "dim");}
          if (i == 11) {publish_controls("14/1", "brighten");}
        } else if (clk4[i] == 1 && rel4[i] == 1) {              //click
          if (i == 0) {publish_controls("12/0", "off");}
          if (i == 1) {publish_controls("12/0", "on");}
          if (i == 2) {publish_controls("12/1", "off");}
          if (i == 3) {publish_controls("12/1", "on");}
          if (i == 4) {publish_controls("13/0", "off");}
          if (i == 5) {publish_controls("13/0", "on");}
          if (i == 6) {publish_controls("13/1", "off");}
          if (i == 7) {publish_controls("13/1", "on");} 
          if (i == 8) {publish_controls("14/0", "off");}
          if (i == 9) {publish_controls("14/0", "on");}
          if (i == 10) {publish_controls("14/1", "off");}
          if (i == 11) {publish_controls("14/1", "on");}   
        } else if (clk4[i] == 2 && rel4[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("12/0", "cool");}
          if (i == 1) {publish_controls("12/0", "heat");}
          if (i == 2) {publish_controls("12/1", "cool");}
          if (i == 3) {publish_controls("12/1", "heat");}
          if (i == 4) {publish_controls("13/0", "cool");}
          if (i == 5) {publish_controls("13/0", "heat");}
          if (i == 6) {publish_controls("13/1", "cool");}
          if (i == 7) {publish_controls("13/1", "heat");}
          if (i == 8) {publish_controls("14/0", "cool");}
          if (i == 9) {publish_controls("14/0", "heat");}
          if (i == 10) {publish_controls("14/1", "cool");}
          if (i == 11) {publish_controls("14/1", "heat");} 
        } //else if (clk4[i] == 2 && rel4[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("12/0", "??");}
         // if (i == 1) {publish_controls("12/0", "??");}
         // if (i == 2) {publish_controls("12/1", "??");}
         // if (i == 3) {publish_controls("12/1", "??");}
         // if (i == 4) {publish_controls("13/0", "??");}
         // if (i == 5) {publish_controls("13/0", "??");}
         // if (i == 6) {publish_controls("13/1", "??");}
         // if (i == 7) {publish_controls("13/1", "??");}
         // if (i == 8) {publish_controls("14/0", "??");}
         // if (i == 9) {publish_controls("14/0", "??");}
         // if (i == 10) {publish_controls("14/1", "??");}
         // if (i == 11) {publish_controls("14/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk4[i] = 0;
      rel4[i] = 0;
      timer4[i] = OFF;
    }

    if (millis() - startMillis5[i] > 500) {
      if (timer5[i] == ON) {
        if (clk5[i] == 1 && rel5[i] == 0) {                     //hold
          if (i == 0) {publish_controls("15/0", "dim");}
          if (i == 1) {publish_controls("15/0", "brighten");}
          if (i == 2) {publish_controls("15/1", "dim");}
          if (i == 3) {publish_controls("15/1", "brighten");}
          if (i == 4) {publish_controls("16/0", "dim");}
          if (i == 5) {publish_controls("16/0", "brighten");}
          if (i == 6) {publish_controls("16/1", "dim");}
          if (i == 7) {publish_controls("16/1", "brighten");}
          if (i == 8) {publish_controls("17/0", "dim");}
          if (i == 9) {publish_controls("17/0", "brighten");}
          if (i == 10) {publish_controls("17/1", "dim");}
          if (i == 11) {publish_controls("17/1", "brighten");}
        } else if (clk5[i] == 1 && rel5[i] == 1) {              //click
          if (i == 0) {publish_controls("15/0", "off");}
          if (i == 1) {publish_controls("15/0", "on");}
          if (i == 2) {publish_controls("15/1", "off");}
          if (i == 3) {publish_controls("15/1", "on");}
          if (i == 4) {publish_controls("16/0", "off");}
          if (i == 5) {publish_controls("16/0", "on");}
          if (i == 6) {publish_controls("16/1", "off");}
          if (i == 7) {publish_controls("16/1", "on");} 
          if (i == 8) {publish_controls("17/0", "off");}
          if (i == 9) {publish_controls("17/0", "on");}
          if (i == 10) {publish_controls("17/1", "off");}
          if (i == 11) {publish_controls("17/1", "on");}   
        } else if (clk5[i] == 2 && rel5[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("15/0", "cool");}
          if (i == 1) {publish_controls("15/0", "heat");}
          if (i == 2) {publish_controls("15/1", "cool");}
          if (i == 3) {publish_controls("15/1", "heat");}
          if (i == 4) {publish_controls("16/0", "cool");}
          if (i == 5) {publish_controls("16/0", "heat");}
          if (i == 6) {publish_controls("16/1", "cool");}
          if (i == 7) {publish_controls("16/1", "heat");}
          if (i == 8) {publish_controls("17/0", "cool");}
          if (i == 9) {publish_controls("17/0", "heat");}
          if (i == 10) {publish_controls("17/1", "cool");}
          if (i == 11) {publish_controls("17/1", "heat");} 
        } //else if (clk5[i] == 2 && rel5[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("15/0", "??");}
         // if (i == 1) {publish_controls("15/0", "??");}
         // if (i == 2) {publish_controls("15/1", "??");}
         // if (i == 3) {publish_controls("15/1", "??");}
         // if (i == 4) {publish_controls("16/0", "??");}
         // if (i == 5) {publish_controls("16/0", "??");}
         // if (i == 6) {publish_controls("16/1", "??");}
         // if (i == 7) {publish_controls("16/1", "??");}
         // if (i == 8) {publish_controls("17/0", "??");}
         // if (i == 9) {publish_controls("17/0", "??");}
         // if (i == 10) {publish_controls("17/1", "??");}
         // if (i == 11) {publish_controls("17/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk5[i] = 0;
      rel5[i] = 0;
      timer5[i] = OFF;
    }

    if (millis() - startMillis6[i] > 500) {
      if (timer6[i] == ON) {
        if (clk6[i] == 1 && rel6[i] == 0) {                     //hold
          if (i == 0) {publish_controls("18/0", "dim");}
          if (i == 1) {publish_controls("18/0", "brighten");}
          if (i == 2) {publish_controls("18/1", "dim");}
          if (i == 3) {publish_controls("18/1", "brighten");}
          if (i == 4) {publish_controls("19/0", "dim");}
          if (i == 5) {publish_controls("19/0", "brighten");}
          if (i == 6) {publish_controls("19/1", "dim");}
          if (i == 7) {publish_controls("19/1", "brighten");}
          if (i == 8) {publish_controls("20/0", "dim");}
          if (i == 9) {publish_controls("20/0", "brighten");}
          if (i == 10) {publish_controls("20/1", "dim");}
          if (i == 11) {publish_controls("20/1", "brighten");}
        } else if (clk6[i] == 1 && rel6[i] == 1) {              //click
          if (i == 0) {publish_controls("18/0", "off");}
          if (i == 1) {publish_controls("18/0", "on");}
          if (i == 2) {publish_controls("18/1", "off");}
          if (i == 3) {publish_controls("18/1", "on");}
          if (i == 4) {publish_controls("19/0", "off");}
          if (i == 5) {publish_controls("19/0", "on");}
          if (i == 6) {publish_controls("19/1", "off");}
          if (i == 7) {publish_controls("19/1", "on");} 
          if (i == 8) {publish_controls("20/0", "off");}
          if (i == 9) {publish_controls("20/0", "on");}
          if (i == 10) {publish_controls("20/1", "off");}
          if (i == 11) {publish_controls("20/1", "on");}   
        } else if (clk6[i] == 2 && rel6[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("18/0", "cool");}
          if (i == 1) {publish_controls("18/0", "heat");}
          if (i == 2) {publish_controls("18/1", "cool");}
          if (i == 3) {publish_controls("18/1", "heat");}
          if (i == 4) {publish_controls("19/0", "cool");}
          if (i == 5) {publish_controls("19/0", "heat");}
          if (i == 6) {publish_controls("19/1", "cool");}
          if (i == 7) {publish_controls("19/1", "heat");}
          if (i == 8) {publish_controls("20/0", "cool");}
          if (i == 9) {publish_controls("20/0", "heat");}
          if (i == 10) {publish_controls("20/1", "cool");}
          if (i == 11) {publish_controls("20/1", "heat");} 
        } //else if (clk6[i] == 2 && rel6[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("18/0", "??");}
         // if (i == 1) {publish_controls("18/0", "??");}
         // if (i == 2) {publish_controls("18/1", "??");}
         // if (i == 3) {publish_controls("18/1", "??");}
         // if (i == 4) {publish_controls("19/0", "??");}
         // if (i == 5) {publish_controls("19/0", "??");}
         // if (i == 6) {publish_controls("19/1", "??");}
         // if (i == 7) {publish_controls("19/1", "??");}
         // if (i == 8) {publish_controls("20/0", "??");}
         // if (i == 9) {publish_controls("20/0", "??");}
         // if (i == 10) {publish_controls("20/1", "??");}
         // if (i == 11) {publish_controls("20/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk6[i] = 0;
      rel6[i] = 0;
      timer6[i] = OFF;
    }

    if (millis() - startMillis7[i] > 500) {
      if (timer7[i] == ON) {
        if (clk7[i] == 1 && rel7[i] == 0) {                     //hold
          if (i == 0) {publish_controls("21/0", "dim");}
          if (i == 1) {publish_controls("21/0", "brighten");}
          if (i == 2) {publish_controls("21/1", "dim");}
          if (i == 3) {publish_controls("21/1", "brighten");}
          if (i == 4) {publish_controls("22/0", "dim");}
          if (i == 5) {publish_controls("22/0", "brighten");}
          if (i == 6) {publish_controls("22/1", "dim");}
          if (i == 7) {publish_controls("22/1", "brighten");}
          if (i == 8) {publish_controls("23/0", "dim");}
          if (i == 9) {publish_controls("23/0", "brighten");}
          if (i == 10) {publish_controls("23/1", "dim");}
          if (i == 11) {publish_controls("23/1", "brighten");}
        } else if (clk7[i] == 1 && rel7[i] == 1) {              //click
          if (i == 0) {publish_controls("21/0", "off");}
          if (i == 1) {publish_controls("21/0", "on");}
          if (i == 2) {publish_controls("21/1", "off");}
          if (i == 3) {publish_controls("21/1", "on");}
          if (i == 4) {publish_controls("22/0", "off");}
          if (i == 5) {publish_controls("22/0", "on");}
          if (i == 6) {publish_controls("22/1", "off");}
          if (i == 7) {publish_controls("22/1", "on");} 
          if (i == 8) {publish_controls("23/0", "off");}
          if (i == 9) {publish_controls("23/0", "on");}
          if (i == 10) {publish_controls("23/1", "off");}
          if (i == 11) {publish_controls("23/1", "on");}   
        } else if (clk7[i] == 2 && rel7[i] == 1) {              //click-hold
          if (i == 0) {publish_controls("21/0", "cool");}
          if (i == 1) {publish_controls("21/0", "heat");}
          if (i == 2) {publish_controls("21/1", "cool");}
          if (i == 3) {publish_controls("21/1", "heat");}
          if (i == 4) {publish_controls("22/0", "cool");}
          if (i == 5) {publish_controls("22/0", "heat");}
          if (i == 6) {publish_controls("22/1", "cool");}
          if (i == 7) {publish_controls("22/1", "heat");}
          if (i == 8) {publish_controls("23/0", "cool");}
          if (i == 9) {publish_controls("23/0", "heat");}
          if (i == 10) {publish_controls("23/1", "cool");}
          if (i == 11) {publish_controls("23/1", "heat");} 
        } //else if (clk7[i] == 2 && rel7[i] == 2) {            //dbl-click
         // if (i == 0) {publish_controls("21/0", "??");}
         // if (i == 1) {publish_controls("21/0", "??");}
         // if (i == 2) {publish_controls("21/1", "??");}
         // if (i == 3) {publish_controls("21/1", "??");}
         // if (i == 4) {publish_controls("22/0", "??");}
         // if (i == 5) {publish_controls("22/0", "??");}
         // if (i == 6) {publish_controls("22/1", "??");}
         // if (i == 7) {publish_controls("22/1", "??");}
         // if (i == 8) {publish_controls("23/0", "??");}
         // if (i == 9) {publish_controls("23/0", "??");}
         // if (i == 10) {publish_controls("23/1", "??");}
         // if (i == 11) {publish_controls("23/1", "??");} 
        //} 
      }
      // Reset variables and timer
      clk7[i] = 0;
      rel7[i] = 0;
      timer7[i] = OFF;
    }

    // Update the previous reading for the next iteration
    mcp0_n1_reading[i] = mcp0_reading[i];
    mcp1_n1_reading[i] = mcp1_reading[i];
    mcp2_n1_reading[i] = mcp2_reading[i];
    mcp3_n1_reading[i] = mcp3_reading[i];
    mcp4_n1_reading[i] = mcp4_reading[i];
    mcp5_n1_reading[i] = mcp5_reading[i];
    mcp6_n1_reading[i] = mcp6_reading[i];
    mcp7_n1_reading[i] = mcp7_reading[i];
  }

  delay(100);
    
  }
}
