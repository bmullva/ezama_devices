 #include <IRremote.h>

int IrReceiverPin = 15;
decode_results results;
IRrecv irrecv(IrReceiverPin);

void setup() {
   Serial.begin(115200);
   pinMode(LED_BUILTIN, OUTPUT);
   Serial.println("Starting IR-receiver...");
 
   irrecv.enableIRIn();
   Serial.println("IR-receiver active");
 
   digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (irrecv.decode(&results)) {
 
     Serial.println(results.value, HEX);
 
     irrecv.resume();
 
     switch (results.value) {
         case 0xFF42BD:  // button *
             digitalWrite(LED_BUILTIN, HIGH);
             break;
 
         case 0xFF52AD:  // button #
             digitalWrite(LED_BUILTIN, LOW);
             break;        
     }
 
   }
 
   delay(100);  

}
