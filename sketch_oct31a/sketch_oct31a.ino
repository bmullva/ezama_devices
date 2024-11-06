const double dVCC = 3.3;             // NodeMCU on board 3.3v vcc
const double dR2 = 10000;            // 10k ohm series resistor
const double dAdcResolution = 1023;  // 10-bit adc

const double dA = 0.001129148;       // thermistor equation parameters
const double dB = 0.000234125;
const double dC = 0.0000000876741; 

void setup() 
{
  Serial.begin(115200);  
}

// ---

void loop() 
{
  double dVout, dRth, dTemperature, dAdcValue; 

  dAdcValue = analogRead(A0);
  dVout = (dAdcValue * dVCC) / dAdcResolution;
  dRth = (dVCC * dR2 / dVout) - dR2;

//  Steinhart-Hart Thermistor Equation:
//  Temperature in Kelvin = 1 / (A + B[ln(R)] + C[ln(R)]^3)
//  where A = 0.001129148, B = 0.000234125 and C = 8.76741*10^-8
  // Temperature in kelvin
  dTemperature = (1 / (dA + (dB * log(dRth)) 
    + (dC * pow((log(dRth)), 3))));   

  // Temperature in degree celsius
  dTemperature = dTemperature - 273.15; 
  dTemperature = (dTemperature * 9/5) + 32;
  Serial.print("Temperature = ");
  Serial.print(dTemperature);
  Serial.println(" degree F");
  delay(500);
}
