/*********
This code does the following:
  Default Access Point is 192.168.4.1
1.1  Serves as an AccessPoint (https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/)
1.1  Collects temperature data from a temperature sensor (see attached file)
1.2  Serves one webpage that provides SETPOINT and TOLERANCE integers
1.3  First page reads setpoint and tolerance from EEPROM, and updates EEPROM
1.3  Second page updates from EEPROM
1.4  GPIO13 is cooling and GPIO 15 is warming
1.4  Set GPIO13 (fan) LOW if GPI13 HIGH && Temp < SETPOINT
1.4  Set GPIO15 (heat) LOW  if GPIO15 HIGH && Temp > SETPOINT 
1.4  Sets GPIO13 HIGH if Temp > SETPOINT + TOLERANCE
1.4  Sets GPIO15 HIGH if Temp < SETPOINT - TOLERANCE
0  Upload W3 CSS & https://www.youtube.com/watch?v=YpzJkolkza0
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include<stdio.h>

const char* ssid     = "EZAMA-Fermentor";
const char* password = "123456789";
int setpoint {}; // temperature int stored in EEPROM(220)
int tolerance {}; // tolerance int, stored in EEPROM(221)
String sysStatus = "HOLDING";

#define ONE_WIRE_BUS 14     // Digital pin connected to the DallasTemp sensor

// Setup the sensor:
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// current temperature, updated in loop()
float t = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates Temp readings every 2 seconds
const long interval = 2000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="5">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .t-label{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 Fermentor</h2>
  <p>
    <span id="sysStatus">%STATUS%</span><br>
	 <span class="t-label">Temperature (&deg;F)</span> 
    <span id="temperature">%TEMPERATURE%</span><br>
    <label class="units" id="setpoint">Setpoint: %SETPOINT%&PlusMinus;%TOLERANCE%&deg;F
    </label>
  </p>
  <button class="units" onclick="window.location.href = '/get';">Set Points</button>
</body>
</html>)rawliteral";

const char index_html2[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .t-label{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 Setpoints</h2>
  <form action="/">
    <label class="t-label" for="setpoint">Setpoint:</label>
    <input class="t-label" type="number" id="setpoint" name="setpoint" min="0" max="100" step="1" value="%SETPOINT%">
    <sup class="units">&deg;F</sup>
    <br><br>
    <label class="t-label" for="tolerance">Tolerance:</label>
    <input class="t-label" type="number" id="tolerance" name="tolerance" min="1" max="10" step="1" value="%TOLERANCE%">
    <sup class="units">&deg;F</sup>
    <br><br>
    <input class="units" type="submit" value="Submit">
  </form>
</body>
<script>


</script>
</html>)rawliteral";


// Replaces placeholder with T values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  //if(var == "STATUS") {return String(sysStatus);}
  if(var == "STATUS" && sysStatus == "COOLING"){
    return String("<i style=\"background-color:blue;color:white\">COOLING</i>");
  }
  if(var == "STATUS" && sysStatus == "WARMING"){
    return String("<i style=\"background-color:red;color:white\">WARMING</i>");
  }
  if(var == "STATUS" && sysStatus == "HOLDING"){
    return String("<i>HOLDING</i>");
  }
  if(var == "STATUS" && sysStatus == ""){
    return String("<i>HOLDING</i>");
  }
  if(var == "SETPOINT"){
    setpoint = EEPROM.read(220);
    return String(setpoint);
  }
  if(var == "TOLERANCE"){
    tolerance = EEPROM.read(221);
    return String(tolerance);
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(14, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  sensors.begin();
  
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  //WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("setpoint") && request->hasParam("tolerance")) {
        String sp = request->getParam("setpoint")->value();
        setpoint = sp.toInt();
        String tol = request->getParam("tolerance")->value();
        tolerance = tol.toInt();
        if (setpoint != EEPROM.read(220) || tolerance != EEPROM.read(221)){
          if (setpoint != EEPROM.read(220)) {EEPROM.write(220, setpoint);}
          if (tolerance != EEPROM.read(221)) {EEPROM.write(221, tolerance);}
          Serial.print("Writing to EEPROM");
          EEPROM.commit();
        }
    }
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/sysStatus", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", sysStatus.c_str());
  });
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html2, processor);
  });

  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the Temp values
    previousMillis = currentMillis;
    // Read temperature as Farenheit
    sensors.requestTemperatures();
    float newT = sensors.getTempFByIndex(0);
    // Read temperature as Celcius
    //float newT = sensors.getTempCByIndex(0);
    // if temperature read failed, don't change t value
    if (isnan(newT) || newT < -190) {
      Serial.println("Failed to read sensor!");
    }
    else {
      t = newT;
      //t = 80;
      int l_set = EEPROM.read(220);
      int l_tol = EEPROM.read(221);
      if (t<l_set && digitalRead(13)==HIGH) {
		    digitalWrite(13, LOW);
		    sysStatus = "HOLDING";
		  }
      if (t>l_set && digitalRead(15)==HIGH) {
		    digitalWrite(15, LOW);
		    sysStatus = "HOLDING";
		  }
      if (t > l_set + l_tol) {
		    digitalWrite(13, HIGH);
		    sysStatus = "COOLING";
		  }
      if (t < l_set - l_tol) {
		    digitalWrite(15, HIGH);
		    sysStatus = "WARMING";
		  }
      Serial.println(t);
      Serial.println(l_set);
      Serial.print(t<l_set);
      Serial.println(digitalRead(13));
      Serial.print(t>l_set);
      Serial.println(digitalRead(15));
    }
  }
}
