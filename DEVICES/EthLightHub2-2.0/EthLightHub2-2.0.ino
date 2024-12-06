#include <ETH.h>          //For WT32-ETH01
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
const char* mqtt_server {};
char mqtt_ip_1[] = "192.168.0.222";
char mqtt_ip_2[] = "192.168.1.222";
char mqtt_ip_3[] = "192.168.4.222";
char device_id[9] = {};
const int mqtt_port = 1883;
const char* mqtt_topic_subscribe = "broadcast";
const char* mqtt_topic_publish = "reporting";
static bool eth_connected = false;
//const int device_id_addr = 222; 
//8 digit (222-229) device_id
const int password_length_addr = 231; // 8 <= len <= 63
// 232-239 NOT USED
const int password_addr = 240; // 8-63 byte (240-302)

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
String type_ = "Ethernet Light Hub";
String ver = "1.0";


Adafruit_MCP23X17 mcp0;
int dim_amt (int dim) {
  return 100 - dim;
}

//VIRTUAL:
//device_id/onOffN: should accept: anything, "on", "off"
//device_id/AConOffN: should accept: anything, "on", "off"
//device_id/luxN: should accept: 0-95
//device_id/tempN: should accept: 0-255
// onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
// lux_array:  lux 0-100 {NA,lux1,lux2,lux3,lux4,lux5,lux6,lux7,lux8,lux9,lux10,lux11,lux12,lux13,lux14};
// temp_array:  {temp13, temp14};
int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temp_array[]   = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 0, 0};


int mom_pin_array[]= {-99,15,0,14,1,13,2,12,3,11,4,10,5};
int low_pins[]  = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 9, 6};
int high_pins[] = {-99,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9, 8, 7};
void t_write(int i) {
  analogWrite(low_pins[i],  dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  analogWrite(high_pins[i], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High  
}


//PHYSICAL:
//device_id/N N=1-12: should accept: "hold", "click", "click-hold", "dbl-click", "release"
//device_id/N N=13,14 should accept "click", "on", "off", "dim", "brighten", "heat", "cool"
//device_id/ACN: should accept: anything, "on", "off"
// +1 -1 or 0: lt_array: NA, lm1,lm2,lm3,lm4,lm5,lm6,lm7,lm8,lm9,lm10,lm11,lm12,lux13,lux14,temp13,temp14 
int lt_array [] = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"]      = type_;
  state_json["ver"]       = ver;
  state_json["IP"]        = WiFi.localIP();
  //state_json["vG"]        = "amp,0,20";
  state_json["vL"]        = "1,14,onOff;1,14,lux;13,14,temp";
  state_json["pL"]        = "1,14,";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  mqttClient.publish(topic.c_str(), sj);
}



// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  //device_id/onOffN: should accept: anything, "on", "off"
  //device_id/luxN: should accept: 0-95
  //device_id/tempN: should accept: 0-255

  for (int i = 1; i<=14; i++) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
  }
  
  for (int i = 1; i<=14; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 13; i<=14; i++) {        
    if (topic == String(device_id) + "/" + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1-12 should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  
  for (int i = 1; i<=12; i++) {        
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

  //device_id/N N=13-14 should accept "on", "off", "dim", "brighten", "heat", "cool", "release"
  //NA, lux13, lux14, temp13, temp14

  for (int i = 13; i<=14; i++) {        
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

  //Take action from non-"sweeping" commands, both virtual and physical.
  //"Sweeping" commands are handled in the main loop.
  for (int i = 1; i<=12; i++) {
    analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }

  for (int i = 13; i<=14; i++) {
    t_write(i);
  }
}


// 5 SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  for (int i = 1; i<=14; i++) {        
    topic = String(device_id)+"/"+String(i);
    mqttClient.subscribe(topic.c_str());
    }

  for (int i = 1; i <= 14; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    mqttClient.subscribe(topic.c_str());
  }

  for (int i = 1; i<=14; i++) {        
    mqttClient.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 13; i<=14; i++) {        
    mqttClient.subscribe((String(device_id)+"/"+String("temp")+String(i)).c_str());
    }
}


void setup() { 
  delay(1000);
  ezama_setup();
  Serial.begin(115200);
  specific_connect();
  
  if (!mcp0.begin_I2C(0x20)) {
    Serial.println("Error initializing MCP23017x0");
    while (1); // Loop forever if there's an error
  }
 
  mcp0.pinMode(0, OUTPUT);
  mcp0.pinMode(1, OUTPUT);
  mcp0.pinMode(2, OUTPUT);
  mcp0.pinMode(3, OUTPUT);
  mcp0.pinMode(4, OUTPUT);
  mcp0.pinMode(5, OUTPUT);
  mcp0.pinMode(8, OUTPUT);
  mcp0.pinMode(9, OUTPUT);
  mcp0.pinMode(10, OUTPUT);
  mcp0.pinMode(11, OUTPUT);
  mcp0.pinMode(12, OUTPUT);
  mcp0.pinMode(13, OUTPUT);
  mcp0.pinMode(14, OUTPUT);
  mcp0.pinMode(15, OUTPUT);

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


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm11Lux, lm12Lux, lm11Temp, lm12Temp

void loop() {
  ezama_loop();  //in ezama.h

  for(int i=1;i<=12;i++) {
    if(lt_array[i] == 1 && lux_array[i] < 99){   // like increasing the dim slider
        lux_array[i] += 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );    
    }
    if(lt_array[i] == -1 && lux_array[i] > 0){  // like decreasing the dim slider
        lux_array[i] -= 1;
        analogWrite(mom_pin_array[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
    }
  }
  
  for(int i=13;i<=14;i++) {  
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
