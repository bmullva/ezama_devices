#include <Wire.h>

#define SLAVE_ADDRESS 0x21
byte switchData[2];
//byte switchData[32];
int switchDataIndex = 0;
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

void requestEvent() {
  Wire.write(switchData, switchDataIndex); // Send array
  memset(switchData, '9', sizeof(switchData)); // Fill the array with nulls
  switchDataIndex = 0;  // Reset the index
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
    pinMode(mom_pins[i], INPUT_PULLUP);;
  }
  
  Wire.begin(SLAVE_ADDRESS); // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event

}

void loop() {
  
  // Loop through an array of digital pins, read their states, and store the readings
  for(int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(mom_pins[i]);
  }

  // Loop through a set of conditions for each of 4 pins
  for (int i = 0; i < 20; i++) {  
    
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
      Serial.println("Detected release on pin");
      addData(sw(i), 4);                                 // release
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
          addData(sw(i), db(sw(i)));
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          addData(sw(i), fo(sw(i)));  
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          addData(sw(i), ch(sw(i))); 
        } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
          addData(sw(i), dbl(sw(i)));
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


void addData(int switch_, int data_) {
  Serial.print("Adding data: Switch ");
  Serial.print(switch_);
  Serial.print(", State ");
  Serial.println(data_);
  switchData[0] = switch_;   // Switch number
    switchData[1] = data_;    // State
}


void addData2(int switch_, int data_) {
  if (switchDataIndex + 2 <= 32) {
  switchData[switchDataIndex++] = switch_;   // Switch number
  switchData[switchDataIndex++] = data_;    // State ('t')
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
