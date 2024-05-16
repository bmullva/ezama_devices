#include "EEPROM.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("start...");
  EEPROM.begin(512);
  
}

void loop() {

  for (int i = 0; i < 512; i++)
    {
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(EEPROM.read(i)); 
      delay(50);
    }
}
