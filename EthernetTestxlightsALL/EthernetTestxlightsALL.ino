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

// VARIABLES
  //VIRTUAL VARIABLES FOR NODE RED UI:
  //from device_id/onOffN: should accept: anything, "on", "off"
  //from device_id/luxN: should accept: 0-95
  //from device_id/tempN: should accept: 0-255
  int simple_lights = 8;
  // onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12};
  // lux_array:  lux 0-100 {NA,lux1,lux2,lux3,lux4,lux5,lux6,lux7,lux8,lux9,lux10,lux11,lux12};
  // temp_array:  {NA,NA,NA,NA,NA,NA,NA,NA,NA,temp9,temp11};
  int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0,-9,0,-9};
  int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0,-9,0,-9};
  int temp_array[]   = {-99,-9,-9,-9,-9,-9,-9,-9,-9, 0,-9,0,-9};

  //arrays:            { NA, 1, 2, 3, 4, 5, 6, 7, 8}  {9,11}  {10,12};
  //int mom_pin_array[]= {-99, 4,13,16,17,18,19,23,25};
  int mom_pins[] =     {-99, 4,13,16,17,18,19,23,25,26,27,32,33};
  //int low_pins[]  =    {-99,-9,-9,-9,-9,-9,-9,-9,-9,26,-9,32,-9};
  //int high_pins[] =    {-99,-9,-9,-9,-9,-9,-9,-9,-9,27,-9,33,-9};

  //PHYSICAL VARIABLES FOR SWITCHES:
  // device_id/N N=1-8 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  // device_id/N (N=9,11) should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
  // lt_array (+1 -1 or 0): NA, lm1,lm2,lm3,lm4,lm5,lm6,lm7,lm8,lux9,temp9,lux11,temp11
  int lt_array [] = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
// SPECIAL FUNCTIONS
int dim_amt (int dim) {
  return 100 - dim;
}

void t_write(int i) {
  //analogWrite(low_pins[i],  dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  //analogWrite(high_pins[i], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High
  analogWrite(mom_pins[i],   dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  analogWrite(mom_pins[i+1], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High 
}

// ESP-NOW structure to send data
typedef struct struct_message {
    uint8_t switchNumber;
    uint8_t switchState;
} struct_message;

struct_message outgoingReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
    Serial.begin(115200);

    setupEthernet();

    EEPROM.begin(512);
    for (int i = 0; i < 8; i++) {
        device_id[i] = char(EEPROM.read(222 + i));
    }

    // ESP-NOW
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(32);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
  
    // Register peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, peerMacAddress, 6); // Add this line
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
  
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }

    // Register send callback
    esp_now_register_send_cb(OnDataSent);

    
    if (connectToMQTT()) {
        client.subscribe(device_id);
        client.subscribe("broadcast");
        client.subscribe("password");
        client.setCallback(callback);
        Serial.println("MQTT connected");
        client.publish("debug", "MQTT connected");
    } else {
        Serial.println("MQTT connection failed");
    }

    String topic {};
    for(int i = 1; i <= SIMPLE_LIGHT_NUM; i++) {
      pinMode(mom_pins[i], OUTPUT);
      client.subscribe((String(device_id) + "/" + String("onOff") + String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
      client.subscribe((String(device_id)+"/"+String(i)).c_str());
    }
    for(int i = SIMPLE_LIGHT_NUM+1; i <= 12; i+=2) {
      pinMode(mom_pins[i], OUTPUT);
      pinMode(mom_pins[i+1], OUTPUT);
      //pinMode(low_pins[i], OUTPUT);
      //pinMode(high_pins[i+1], OUTPUT);
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

  for(int i=1;i<=SIMPLE_LIGHT_NUM;i++) {
    if(lt_array[i] == 1 && lux_array[i] < 99){   // like increasing the dim slider
        lux_array[i] += 1;
        analogWrite(mom_pins[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );    
    }
    if(lt_array[i] == -1 && lux_array[i] > 0){  // like decreasing the dim slider
        lux_array[i] -= 1;
        analogWrite(mom_pins[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
    }
  }
  
  for(int i=SIMPLE_LIGHT_NUM+1;i<=12;i+=2) {  
    if (lt_array[i] == 1 && lux_array[i] < 95) {  // like increasing the dim slider
       lux_array[i] += 1;
       t_write(i);
    }    
    if(lt_array[i] == -1 && lux_array[i] > 0)  {  // like decreasing the dim slider
        lux_array[i] -= 1;
        t_write(i);
    }
    if(lt_array[i+2] == 1 && temp_array[i] < 255)  {   // like increasing the temp slider
        temp_array[i] += 5;
        if (temp_array[i] >= 255) {temp_array[i] = 255;}
        t_write(i);
    }
    if(lt_array[i+2] == -1 && temp_array[i] > 0){  // like decreasing the temp slider
        temp_array[i] -= 5;
        if (temp_array[i] <= 0) {temp_array[i] = 0;}
        t_write(i);
    }
  }        

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
    } else {
        receive_controls_json(topic, message);
        client.publish("debug", "Sent to receive controls json");
    }
}

void receive_controls_json(String topic, String msg) {    
  // VIRTUAL MSGS: 
  // device_id/onOffN: should accept: anything, "on", "off"
  // device_id/luxN: should accept: 0-95
  // device_id/tempN: should accept: 0-255

  String debugMsg = "receive controls - " + topic + " " + msg;
  client.publish("debug", debugMsg.c_str());
  for (int i = 1; i<=SIMPLE_LIGHT_NUM; i++) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }

  for (int i = SIMPLE_LIGHT_NUM+1; i<=12; i+=2) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }
  

  //PHYSICAL MSGS
  //device_id/N N=1-12 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  //device_id/N N=13-14 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //NA, lux13, lux14, temp13, temp14

  for (int i = 1; i<=SIMPLE_LIGHT_NUM; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {

        if (msg == "hold") {
          lt_array[i] = 1;
        }
        
        if (msg == "click") {
          onOff_array[i] = 1 - onOff_array[i];
        }

        if (msg == "click-hold") {
          lt_array[i] = -1;
        }
        
        if (msg == "dbl-click") {
          
        }

        if (msg == "on") {
          onOff_array[i] = 1;
        }
        
        if (msg == "off") {
          onOff_array[i] = 0;    
        }

        if (msg == "dim") {
          lt_array[i] = 1;
        }

        if (msg == "brighten") {
          lt_array[i] = -1;
        }
        
        if (msg == "release") {
          lt_array[i] = 0;
        }        
    }
  }

  for (int i = SIMPLE_LIGHT_NUM+1; i<=12; i+=2) {        
    if (topic == String(device_id) + "/" + String(i)) {
        // msg to these topics should be "click", "on", "off", "dim", "brighten", "heat", "cool", "release"
               
        if (msg == "click") {
          onOff_array[i] = 1 - onOff_array[i];
        }
        
        if (msg == "on") {
          onOff_array[i] = 1;
        }

        if (msg == "off") {
          onOff_array[i] = 0;
        }
        
        if (msg == "dim") {
          lt_array[i] = 1;
        }

        if (msg == "brighten") {
          lt_array[i] = -1;
        }

         if (msg == "heat") {
          lt_array[i+2] = 1;
        }
        
        if (msg == "cool") {
          lt_array[i+2] = -1;
        }

        if (msg == "release") {
          lt_array[i] = 0;
          lt_array[i+2] = 0;
        }
    }
  }

  // TAKE ACTION on onOff_array, lux_array, and temp_array 
  // Sweeping" commands are handled in the main_loop
  // Sweeping commands includes "dim", "brighten", "heat", "cool", "release"
  for (int i = 1; i<=SIMPLE_LIGHT_NUM; i++) {
    String debugMsg = String(mom_pins[i]) + ":" + String(dim_amt(lux_array[i]) * 255 * onOff_array[i] / 100);
    client.publish("debug", debugMsg.c_str());
    analogWrite(mom_pins[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }
  for (int i = SIMPLE_LIGHT_NUM+1; i<=12; i+=2) {
    t_write(i);
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
  //state_json["vL"]        = "1,4,onOff;1,4,lux;1,4,temp";
  //state_json["pL"]        = "1,4,";
  state_json["pS"]= "0,1,";

  serializeJson(state_json, output);
  client.publish("reporting", output.c_str());
}
