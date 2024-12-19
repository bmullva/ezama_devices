#include <ModbusRTU.h>

#define MODBUS_SERIAL Serial2
#define MODBUS_RX 21
#define MODBUS_TX 22
#define SLAVE_ID 1  // Adjust this for each slave; should correspond to SLAVE_ADDR_1 in master code

ModbusRTU mb;

// Arrays for button state tracking
int d_pin_reading[20] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
int d_pin_n1_reading[20] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long startMillis[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int clk[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rel[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int mom_pins[20] = {2, 4, 5, 13, 14, 15, 16, 17, 18, 19, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};

enum ButtonState {
  OFF,
  ON
};
ButtonState timer[20] = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < sizeof(mom_pins) / sizeof(mom_pins[0]); i++) {
    pinMode(mom_pins[i], INPUT_PULLUP);
  }

  // Initialize Modbus on custom pins
  MODBUS_SERIAL.begin(9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX);
  mb.begin(&MODBUS_SERIAL);
  mb.setBaudrate(9600);
  mb.slave(SLAVE_ID);

  for (int i = 0; i < 20; i++) {
    mb.addHreg(i, 0); // Initialize each holding register to 0
  }
}

void loop() {
  // Loop through an array of digital pins, read their states, and store the readings
  for (int i = 0; i < sizeof(d_pin_reading) / sizeof(d_pin_reading[0]); i++) {
    d_pin_reading[i] = digitalRead(mom_pins[i]);
  }

  // Loop through conditions for each of 20 pins
  for (int i = 0; i < 20; i++) {
      
    if (d_pin_n1_reading[i] == LOW && d_pin_reading[i] == HIGH && timer[i] == OFF) {
      Serial.println("Detected release on pin");
      sendData(sw(i), 4);                                 // release
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
          sendData(sw(i), db(sw(i)));
        } else if (clk[i] == 1 && rel[i] == 1) {              //click
          sendData(sw(i), fo(sw(i)));  
        } else if (clk[i] == 2 && rel[i] == 1) {              //click-hold
          sendData(sw(i), ch(sw(i))); 
        } else if (clk[i] == 2 && rel[i] == 2) {              //dbl-click
          sendData(sw(i), dbl(sw(i)));
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

// Helper functions remain the same
int sw(int x) {
  return x / 2;
}

int db(int x) {
  if (x % 2 == 0) { 
    return 2;  // dim
  }
  if (x % 2 != 0) {
    return 3;  // brighten
  }
}

int fo(int x) {
  if (x % 2 == 0) { 
    return 1;  // off
  }
  if (x % 2 != 0) {
    return 0;  // on
  }
}

int ch(int x) {
  if (x % 2 == 0) { 
    return 6;    // cool
  }
  if (x % 2 != 0) {
    return 5;    // heat
  }
}

int dbl(int x) {
  if (x % 2 == 0) { 
    return 8;    // double off
  }
  if (x % 2 != 0) {
    return 7;    // double on
  }
}

void sendData(int switch_, int data_) {
  if (switch_ < 20) {  // Assuming 20 switches based on your previous code
    Serial.print("Sending data: Switch ");
    Serial.print(switch_);
    Serial.print(", State ");
    Serial.println(data_);
    
    uint16_t dataToSend = (switch_ << 8) | data_;
    mb.Hreg(switch_, dataToSend);  // Use this method to set the register value
    mb.task();  // Ensure the change is processed
  } else {
    Serial.println("Error: Invalid switch number");
  }
}

// Note: The addData and addData2 functions are removed as they are no longer needed with Modbus communication.
