#include <PJONSoftwareBitBang.h>  // For ESP32 and PJON Comms

#define PJON_DATA_PIN 21
PJONSoftwareBitBang bus(1);       //PJON device number UNIQUE FOR ESP

int d_pin_reading [20]         = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
int d_pin_n1_reading [20]      = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
unsigned long startMillis [20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int clk [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int rel [20]                   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int mom_pins [20] = {2,4,5,13,14,15,16,17,18,19,23,25,26,27,32,33,34,35,36,39};


enum ButtonState {
  OFF,
  ON
};
ButtonState timer[20]          = {OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF};

void setup() {
  Serial.begin(115200);
  bus.strategy.set_pin(PJON_DATA_PIN);
  bus.begin();
  
  for(int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
    pinMode(mom_pins[i], INPUT_PULLUP);
  }
}

void loop() {
  
  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(mom_pins[i]);
  }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 20; i++) {  
    
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
      int switch_ = sw(i);
      int data_ = 4;
      uint8_t combinedData[2] = {switch_, data_};
      bus.send(99, (const char*)combinedData, 2);                       // release
      Serial.println("Sent Release");
      Serial.print("Sending message: ");
      for (int i = 0; i < 2; i++) {  // Only iterate over the 2 bytes you're sending
        Serial.print(combinedData[i], HEX);  // Print each byte in hexadecimal
        Serial.print(" ");
      }
      Serial.println();
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
        if (clk[i] == 1 && rel[i] == 0) {                     //hold
          int switch_ = sw(i);
          int data_ = db(sw(i));   /* appropriate state function like fo, db, ch, dbl */
          uint8_t combinedData[2] = {switch_, data_};
          bus.send(99, (const char*)combinedData, 2);
          Serial.println("Sent hold data");
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          int switch_ = sw(i);
          int data_ = fo(sw(i));   /* appropriate state function like fo, db, ch, dbl */
          uint8_t combinedData[2] = {switch_, data_};
          bus.send(99, (const char*)combinedData, 2);
          Serial.println("Sent click data");
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          int switch_ = sw(i);
          int data_ = ch(sw(i));   /* appropriate state function like fo, db, ch, dbl */
          uint8_t combinedData[2] = {switch_, data_};
          bus.send(99, (const char*)combinedData, 2);
          Serial.println("Sent click-hold data");
        } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
          int switch_ = sw(i);
          int data_ = dbl(sw(i));   /* appropriate state function like fo, db, ch, dbl */
          uint8_t combinedData[2] = {switch_, data_};
          bus.send(99, (const char*)combinedData, 2);
          Serial.println("Sent dbl-click data");
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
  return (x+20)/2;
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
