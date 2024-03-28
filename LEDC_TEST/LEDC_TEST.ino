/*
  Fading

  This example shows how to fade an LED using the analogWrite() function.

  The circuit:
  - LED attached from digital pin 9 to ground through 220 ohm resistor.

  created 1 Nov 2008
  by David A. Mellis
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Fading
*/

//int setting = 1;    // ledc
int setting = 0;    // analogWrite
void setup() {
  if (setting == 1) {
    //ledcSetup(0, 5000, 8);
  
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);
  ledcSetup(4, 5000, 8);
  ledcSetup(5, 5000, 8);
  ledcSetup(6, 5000, 8);
  ledcSetup(7, 5000, 8);
  ledcSetup(8, 5000, 8);
  ledcSetup(9, 5000, 8);
  ledcSetup(10, 5000, 8);
  ledcSetup(11, 5000, 8);
  ledcSetup(12, 5000, 8);
  ledcSetup(13, 5000, 8);
  ledcSetup(14, 5000, 8);
  ledcSetup(15, 5000, 8);

  //ESP32 Left Hand Side  I have set the ledc numbers to match the board numbers
  //GPI36
  //GPI39
  //GPI34
  //GPI35
  ledcAttachPin(32, 14);  //LED13Low
  ledcAttachPin(33, 11);  //LED11
  ledcAttachPin(25, 9);   //LED09
  ledcAttachPin(26, 7);   //LED07
  ledcAttachPin(27, 5);   //LED05
  pinMode(14, OUTPUT);    //AC3
  //pinMode(12, OUTPUT);    //AC1
  ledcAttachPin(13, 3);   //LED03

  //ESP32 Right Hand Side
  ledcAttachPin(23, 1);   //LED01
  pinMode(22, OUTPUT);
  //ledcAttachPin(22, 0);   //LED14Low
  //GPIO1
  //GPIO3
  ledcAttachPin(21, 12);  //LED12
  ledcAttachPin(19, 10);  //LED10
  ledcAttachPin(18, 8);   //LED08
  ledcAttachPin(5, 13);   //LED13High
  ledcAttachPin(17, 6);   //LED06
  ledcAttachPin(16, 4);   //LED04
  ledcAttachPin(4, 2);    //LED02
  pinMode(2, OUTPUT);     //AC2
  //ledcAttachPin(15, 15);  //LED14High
  //pinMode(15, OUTPUT);
  }

if (setting == 0) {
    //ledcSetup(0, 5000, 8);
  
  //ESP32 Left Hand Side  I have set the ledc numbers to match the board numbers
  //GPI36
  //GPI39
  //GPI34
  //GPI35
  pinMode(32, OUTPUT);  //LED13Low
  pinMode(33, OUTPUT);  //LED11
  pinMode(25, OUTPUT);   //LED09
  pinMode(26, OUTPUT);   //LED07
  pinMode(27, OUTPUT);   //LED05
  pinMode(14, OUTPUT);    //AC3
  //pinMode(12, OUTPUT);    //AC1
  pinMode(13, OUTPUT);   //LED03

  //ESP32 Right Hand Side
  pinMode(23, OUTPUT);   //LED01
  pinMode(22, OUTPUT);
  //ledcAttachPin(22, 0);   //LED14Low
  //GPIO1
  //GPIO3
  pinMode(21, OUTPUT);  //LED12
  pinMode(19, OUTPUT);  //LED10
  pinMode(18, OUTPUT);   //LED08
  pinMode(5, OUTPUT);   //LED13High
  pinMode(17, OUTPUT);   //LED06
  pinMode(16, OUTPUT);   //LED04
  pinMode(4, OUTPUT);    //LED02
  pinMode(2, OUTPUT);     //AC2
  //ledcAttachPin(15, 15);  //LED14High
  //pinMode(15, OUTPUT);
  }
  
}

void loop() {
  // fade in from min to max in increments of 5 points:
  
  
  digitalWrite(2, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(22, HIGH);
  delay(2000);
  
  for (int i = 1; i < 15; i++) {
    for (int fadeValue1 = 0 ; fadeValue1 <= 255; fadeValue1 += 5) {
      if (setting == 1) {
        ledcWrite(i, fadeValue1);
      }
      else {
        analogWrite(i, fadeValue1);
      }
      delay(30);
    }
  }
  delay(4000);
  for (int i = 1; i < 15; i++) {
    for (int fadeValue2 = 255 ; fadeValue2 >= 0; fadeValue2 -= 5) {
      // sets the value (range from 0 to 255):
      if (setting == 1) {
        ledcWrite(i, fadeValue2);
      }
      else {
        analogWrite(i, fadeValue2);
      }
      delay(30);
    }
  }
  delay(2000);
  digitalWrite(2, LOW);
  digitalWrite(14, LOW);
  digitalWrite(22, LOW);
  delay(2000);
}
