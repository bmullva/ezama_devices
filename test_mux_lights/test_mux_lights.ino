// Pin definitions for the WT32-ETH01 connected to the CD74HC4067
const int outputPin = 4;  // OUTPUT pin of WT32-ETH01
const int s0 = 17;        // S0 of MUX
const int s1 = 5;         // S1 of MUX
const int s2 = 33;        // S2 of MUX
const int s3 = 32;        // S3 of MUX

// Global variables for brightness control
int brightness = 0;    // how bright the LEDs are
int fadeAmount = 20;    // how many points to fade the LEDs by
unsigned long previousFadeMillis = 0;  // Stores last time brightness was updated
const long fadeInterval = 1000;  // Interval at which to update brightness, in milliseconds

void setup() {
  // Initialize the output pins
  pinMode(outputPin, OUTPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  
  // Start with all control pins at LOW to select channel 0 (not used here)
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  
  setChannel(1); // Try setting to channel 1 explicitly
  analogWrite(outputPin, 255); // Full brightness to test if it lights up
  delay(1000); // Wait to see if it works
  analogWrite(outputPin, 0);

  // Fade from off to full brightness
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    analogWrite(outputPin, fadeValue);
    delay(30); // Wait for 30ms to see the dimming effect
  }

  // Fade from full brightness to off
  for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
    analogWrite(outputPin, fadeValue);
    delay(30); // Wait for 30ms to see the dimming effect
  }

  // Set brightness to 0 after the fade cycle
  analogWrite(outputPin, 0);
  delay(30);
}

void loop() {
  unsigned long currentMillis = millis();

  // Update brightness every 30ms
  if (currentMillis - previousFadeMillis >= fadeInterval) {
    previousFadeMillis = currentMillis;

    // Update brightness for the next iteration
    brightness = brightness + fadeAmount;
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  }

  // Apply brightness to all channels without blocking
  for (int i = 1; i <= 12; i++) {
    setChannel(i);
    analogWrite(outputPin, brightness);
    delay(2);
  }
  
  // Here you can add other non-blocking tasks if needed
}

// Function to select the channel of the multiplexer
void setChannel(int channel) {
  int s0State = bitRead(channel, 0);
  int s1State = bitRead(channel, 1);
  int s2State = bitRead(channel, 2);
  int s3State = bitRead(channel, 3);

  digitalWrite(s0, s0State);
  digitalWrite(s1, s1State);
  digitalWrite(s2, s2State);
  digitalWrite(s3, s3State);
}
