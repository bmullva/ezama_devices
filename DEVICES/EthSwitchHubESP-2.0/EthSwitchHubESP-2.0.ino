#include <WiFi.h>  // For ESP32 and ESP-NOW
#include <esp_now.h>
#include <esp_wifi.h>

#define BOARD_NUMBER 3
#define PJON_DATA_PIN 21
uint8_t peerMacAddress[] = {0xA0, 0xB7, 0x65, 0x2D, 0x25, 0xD8};

int d_pin_reading [20]         = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
int d_pin_n1_reading [20]      = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
unsigned long startMillis [20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int clk [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int rel [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int mom_pins [20] = {2,4,5,13,14,15,16,17,18,19,23,25,26,27,32,33,34,35,36,39};
int mom_pins [20] = {2,4,5,13,14,15,16,17,18,19,23,25,26,27,32,33};

enum ButtonState {
  OFF,
  ON
};
ButtonState timer[20]          = {OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF};

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
  
  // Set device as a Wi-Fi Station
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

  for(int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
    pinMode(mom_pins[i], INPUT_PULLUP);
  }
}

void loop() {
    // Loop through an array of digital pins, read their states, and store the readings

    for(int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
       d_pin_reading[i] = digitalRead(mom_pins[i]);
      
      if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
            outgoingReadings.switchNumber = sw(i);
            outgoingReadings.switchState = 4;  // release
            esp_err_t result = esp_now_send(peerMacAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
            if (result == ESP_OK) {
                Serial.println("Sent with success");
            } else {
                Serial.println("Error sending the data");
            }
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
            Serial.println("Second press on pin");
            clk[i] += 1;
        }
        
        // Check for timer expiration
        if (millis() - startMillis[i] > 500) {
            if (timer[i] == ON) {
                outgoingReadings.switchNumber = sw(i);
                if (clk[i] == 1 && rel[i] == 0) {                     //hold
                    outgoingReadings.switchState = db(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 1 && rel[i] == 1) {              //click
                    outgoingReadings.switchState = fo(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
                    outgoingReadings.switchState = ch(i);   /* appropriate state function like fo, db, ch, dbl */
                } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
                    outgoingReadings.switchState = dbl(i);   /* appropriate state function like fo, db, ch, dbl */
                } 
                esp_err_t result = esp_now_send(peerMacAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
                if (result == ESP_OK) {
                    Serial.println("Sent with success");
                } else {
                    Serial.println("Error sending the data");
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

    delay(75);
}

int sw(int x) {
  return (x+BOARD_NUMBER*20)/2;
}

int db(int x) {
  if (x%2 == 0) { 
    return 2;  // dim
  }
  if (x%2 != 0) {
    return 3;  // brighten
  }
}

int fo(int x) {
  if (x%2 == 0) { 
    return 1;  // off
  }
  if (x%2 != 0) {
    return 0;  // on
  }
}

int ch(int x) {
  if (x%2 == 0) { 
    return 6;    // cool
  }
  if (x%2 != 0) {
    return 5;    // heat
  }
}

int dbl(int x) {
  if (x%2 == 0) { 
    return 8;    // double off
  }
  if (x%2 != 0) {
    return 7;    // double on
  }
}

/*
          case 0x00: state = "on"; break;
          case 0x01: state = "off"; break;
          case 0x02: state = "dim"; break;
          case 0x03: state = "brighten"; break;
          case 0x04: state = "release"; break;
          case 0x05: state = "heat"; break;
          case 0x06: state = "cool"; break;
          case 0x07: state = "dbon"; break;
          case 0x08: state = "dboff"; break;
 */
