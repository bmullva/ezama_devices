#include <WiFi.h>   //For ESP32 and ESP-NOW Comms
#include <esp_now.h>
#include <esp_wifi.h>

#define SIMPLE_LIGHT_NUM 8

// VARIABLES
  //VIRTUAL VARIABLES FOR NODE RED UI:
  //from device_id/onOffN: should accept: anything, "on", "off"
  //from device_id/luxN: should accept: 0-95
  //from device_id/tempN: should accept: 0-255
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
    char topic[50]; // Should match the sending side
    char message[50]; // Should match the sending side
} struct_message;

struct_message incomingData;

void setup() {
    Serial.begin(115200);

    // ESP-NOW
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(32);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    } else {
      Serial.println("ESP-NOW initialized");
    }
    
    uint8_t channel;
    esp_wifi_get_channel(&channel, NULL);
    Serial.print("Current WiFi Channel: ");
    Serial.println(channel);

    uint32_t version;
    if (esp_now_get_version(&version) == ESP_OK) {
        Serial.print("ESP-NOW Version: ");
        Serial.println(version, HEX);
    } else {
        Serial.println("Failed to get ESP-NOW version");
    }

    // Print MAC Address
    Serial.print("ESP32 MAC Address: ");
    Serial.println(WiFi.macAddress());

    esp_now_register_recv_cb(OnDataRecv);

    for(int i = 1; i <= 12; i++) {
      pinMode(mom_pins[i], OUTPUT);
    }
}


void setup2() {
    Serial.begin(115200);

    // ESP-NOW
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(32);
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }  else {
      Serial.println("ESP-NOW initialized");
    }
    
    esp_wifi_set_channel(0, WIFI_SECOND_CHAN_NONE);
    esp_now_register_recv_cb(OnDataRecv);

    for(int i = 1; i <= 12; i++) {
      pinMode(mom_pins[i], OUTPUT);
    }
}


void loop() {
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


void OnDataRecv(const uint8_t * mac, const uint8_t *receivedData, int len) {
  memcpy(&incomingData, receivedData, len);
  String topic = String(incomingData.topic);
  String msg = String(incomingData.message);
  
  // VIRTUAL MSGS: 
  // device_id/onOffN: should accept: anything, "on", "off"
  // device_id/luxN: should accept: 0-95
  // device_id/tempN: should accept: 0-255

  String debugMsg = "receive controls - " + topic + " " + msg;
  Serial.println(debugMsg.c_str());
  for (int i = 1; i<=SIMPLE_LIGHT_NUM; i++) {        
    if (topic.substring(8) == String("/") + "onOff" + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
    if (topic.substring(8) == String("/") + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }

  for (int i = SIMPLE_LIGHT_NUM+1; i<=12; i+=2) {        
    if (topic.substring(8) == String("/") + "onOff" + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
      if (msg == "on") {
        onOff_array[i] = 1;
      } 
      if (msg == "off") {
        onOff_array[i] = 0;    
      }
    }
    if (topic.substring(8) == String("/") + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
    if (topic.substring(8) == String("/") + "temp" + String(i)) {
      temp_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }
  

  //PHYSICAL MSGS
  //device_id/N N=1 to SIMPLE_LIGHTS_NUM should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  //device_id/N N=SIMPLE_LIGHTS_NUM+1,3,.. should accept "on", "off", "dim", "brighten", "heat", "cool", "release"

  for (int i = 1; i<=SIMPLE_LIGHT_NUM; i++) {        
    if (topic.substring(8) == "/" + String(i)) {

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
    if (topic.substring(8) == "/" + String(i)) {
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
    Serial.println(debugMsg.c_str());
    analogWrite(mom_pins[i], dim_amt(lux_array[i]) * 255 * onOff_array[i] /100 );
  }
  for (int i = SIMPLE_LIGHT_NUM+1; i<=12; i+=2) {
    t_write(i);
  }
}
