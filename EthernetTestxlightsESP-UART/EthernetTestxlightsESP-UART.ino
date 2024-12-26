#include <HardwareSerial.h>

#define SIMPLE_LIGHT_NUM 8
#define UART_TX_PIN 22  // GPIO22 for clock (TX)
#define UART_RX_PIN 21  // GPIO21 for data (RX)

// VARIABLES
  //VIRTUAL VARIABLES FOR NODE RED UI:
  //from device_id/onOffN: should accept: anything, "on", "off"
  //from device_id/luxN: should accept: 0-95
  //from device_id/tempN: should accept: 0-255
  // onOff_array: onOff 0, 1: {NA, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12};
  // lux_array:  lux 0-100 {NA,lux1,lux2,lux3,lux4,lux5,lux6,lux7,lux8,lux9,lux10,lux11,lux12};
  // temp_array:  {NA,NA,NA,NA,NA,NA,NA,NA,NA,temp9,temp11};
int mom_pins[]     = {-99, 4,13,16,17,18,19,23,25,26,27,32,33};
int onOff_array[]  = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0,-9,0,-9};
int lux_array[]    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0,-9,0,-9};
int temp_array[]   = {-99,-9,-9,-9,-9,-9,-9,-9,-9, 0,-9,0,-9};
int lt_array []    = {-99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int dim_amt (int dim) {
  return 100 - dim;
}

void t_write(int i) {
  analogWrite(mom_pins[i],   dim_amt(lux_array[i]) * (255-temp_array[i]) * onOff_array[i] /100 );  // Low
  analogWrite(mom_pins[i+1], dim_amt(lux_array[i]) *     temp_array[i]   * onOff_array[i] /100 );  // High 
}

// Structure for UART communication
struct struct_message {
    char topic[50];
    char message[50];
} incomingData;

HardwareSerial UART_custom(2); // Use UART2 for communication

void setup() {
    Serial.begin(115200);
    UART_custom.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN); // RX first for ESP32

    for(int i = 1; i <= 12; i++) {
      pinMode(mom_pins[i], OUTPUT);
    }
}

void loop() {
    if (UART_custom.available() >= sizeof(incomingData)) {
        UART_custom.readBytes((uint8_t*)&incomingData, sizeof(incomingData));
        OnDataRecv();
    }

    // Existing loop for handling light commands
    for(int i=1;i<=SIMPLE_LIGHT_NUM;i++) {
        if(lt_array[i] == 1 && lux_array[i] < 95){   // like increasing the dim slider
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

void OnDataRecv() {
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
  

  // PHYSICAL MSGS
  // device_id/N N=1 to SIMPLE_LIGHTS_NUM should accept "click", "hold", "click-hold", "dbl-click", "release", "on", "off", "dim", "brighten"
  // device_id/N N=SIMPLE_LIGHTS_NUM+1,3,.. should accept "on", "off", "dim", "brighten", "heat", "cool", "release"

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
