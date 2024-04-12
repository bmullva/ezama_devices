
#include <WiFi.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

const char* WIFI_NAME; 
const char* WIFI_PASSWORD; 

WiFiServer server(80);

TaskHandle_t Task1;
TaskHandle_t Task2;

//const int GPIO22   = 22;
const int GPIO21   = 21;
const int GPIO19   = 19;
const int GPIO18   = 18;
const int GPIO17   = 17;
const int GPIO16   = 16;

#define CONNECTION_TIMEOUT 10

//NodeMCU has weird pin mapping.
//Pin numbers written on the board itself do not correspond to ESP8266 GPIO pin numbers. We have constants defined to make using this board easier:

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

int bufLength;
String current_data_line = "";
char new_byte;

String header;
char tmpstr[10];

String LED1STATE = "off";
String LED2STATE = "off";
String LED3STATE = "off";

int GPIO22 = 22;
int GPIO23 = 23;
int GPIO15 = 15;

WiFiClient client;
//************************************************************
void createtask()
{
   xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 
}
//**************************************************************
String get_wifi_status(int status)
{
    switch(status)
    {
        case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
        break;
        case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
        break;
        case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
        break;
        case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
        break;
        case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
        break;
        case WL_CONNECTED:
        return "WL_CONNECTED";
        break;
        case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
        break;
    }
    return "success";
}
//*************************************************************
void connectwifi()
{
  Serial.print("Connecting to ");
  Serial.println(WIFI_NAME);
  getpass();
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  int timeout_counter = 0;
  int status;
  
  while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(200);
        status = WiFi.status();
        Serial.println(get_wifi_status(status));
        timeout_counter++;
        if(timeout_counter >= CONNECTION_TIMEOUT*5)
        {
           Serial.println("Net failed - restarting the ESP32");
           delay(5000);
           ESP.restart();
        }
     }
  Serial.println("");
  Serial.println("Successfully connected to WiFi network");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
//***********************************************************
void setup() 
{
  Serial.begin(115200);
 
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
  Serial.begin(115200);
  pinMode(GPIO22, OUTPUT);
  pinMode(GPIO23, OUTPUT);
  pinMode(GPIO15, OUTPUT);

  digitalWrite(GPIO22, LOW);
  digitalWrite(GPIO23, LOW);
  digitalWrite(GPIO15, LOW);
  
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  createtask();
  connectwifi();
}
//*********************************************************************
void Task1code( void * pvParameters )
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;)
    {
       if (Serial.available()) 
       {
         SerialBT.write(Serial.read());
       }
      
      if (SerialBT.available()) 
      {
        char incomingByte =  char (SerialBT.read()) ;
        
        if (incomingByte == '1')
           { digitalWrite(GPIO22,HIGH); }
        else
        if (incomingByte == '0')
           { digitalWrite(GPIO22,LOW);  }

        if (incomingByte == '3')
           { digitalWrite(GPIO23,HIGH); }
        else
        if (incomingByte == '2')
           { digitalWrite(GPIO23,LOW);  }
        Serial.write(SerialBT.read());
      }
      delay(20);
    } 
}
//**********************************************************************
//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters )
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  if (WiFi.status() == WL_CONNECTED )
      {
        Serial.println("WIFI connected");
      }
  else
      {
        //ESP.restart();
      }
      
  for(;;)
  {
        client = server.available(); 
        if (client) 
          { 
            Serial.println("New Client is requesting web page"); 
            String current_data_line = ""; 
            while (client.connected()) 
              { 
                if (client.available())
                  { 
                    char new_byte = client.read(); 
                    Serial.write(new_byte); 
                    header += new_byte;
                    if (new_byte == '\n') 
                      { 
               
                        if (current_data_line.length() == 0) 
                            {
                  
                              client.println("HTTP/1.1 200 OK");
                              client.println("Content-type:text/html");
                              client.println("Connection: close");
                              client.println();
                  
                              if (header.indexOf("LED0=ON") != -1) 
                                 {
                                    Serial.println("GPIO23 LED is ON");
                                    LED1STATE = "on";
                                    digitalWrite(GPIO22, HIGH);
                                 } 
                              if (header.indexOf("LED0=OFF") != -1) 
                                 {
                                    Serial.println("GPIO23 LED is OFF");
                                    LED1STATE = "off";
                                    digitalWrite(GPIO22, LOW);
                                  } 
                              if (header.indexOf("LED1=ON") != -1)
                                  {
                                    Serial.println("GPIO23 LED is ON");
                                    LED2STATE = "on";
                                    digitalWrite(GPIO23, HIGH);
                                  }
                              if (header.indexOf("LED1=OFF") != -1) 
                                  {
                                    Serial.println("GPIO23 LED is OFF");
                                    LED2STATE = "off";
                                    digitalWrite(GPIO23, LOW);
                                  }
                              if (header.indexOf("LED2=ON") != -1) 
                                  {
                                    Serial.println("GPIO15 LED is ON");
                                    LED3STATE = "on";
                                    digitalWrite(GPIO15, HIGH);
                                  }
                              if(header.indexOf("LED2=OFF") != -1) 
                                  {
                                    Serial.println("GPIO15 LED is OFF");
                                    LED3STATE = "off";
                                    digitalWrite(GPIO15, LOW);
                                  }
                  
                              client.println("<!DOCTYPE html><html>");
                              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                              client.println("<link rel=\"icon\" href=\"data:,\">");
                              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                              client.println(".button { background-color: #4CAF50; border: 2px solid #4CAF50;; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
                              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"); 
                              // Web Page Heading
                              client.println("</style></head>");
                              client.println("<body><center><h1 style='text-Shadow:2px 2px 2px blue;'>ESP32 Web server LED control</h1></center>");
                              client.println("<center><h2 style='font-size:18px;'>Demo of WIFI and Bluetooth control</h2></center>" );
                              client.println("<center><h3 style='font-size:16px;'>Press ON and OFF button to turn no and turn off LEDs</h3></center>");
                              client.println("<form ><center>");
                              client.println("<fieldset style='border: 2px solid #F88622; width: 555px;height:400px;'>");
                              client.println("<p> LED one is " + LED1STATE + "</p>");
                              // If the PIN_NUMBER_22State is off, it displays the ON button 
                              
                              if (LED1STATE == "on")
                                 {
                                   client.println("<center> <button class=\"button\" name=\"LED0\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED0 ON</button>") ;
                                   client.println("<button class=\"button\" name=\"LED0\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED0 OFF</button><br><br>");
                                 }
                              else 
                                 { 
                                  client.println("<center> <button class=\"button\" name=\"LED0\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED0 ON</button>") ;     
                                  client.println("<button class=\"button\" name=\"LED0\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED0 OFF</button><br><br>");
                                 }  
                                 
                              client.println("<p>LED two is " + LED2STATE + "</p>");                              
                              if (LED2STATE == "on")
                                 {
                                   client.println("<center> <button class=\"button\" name=\"LED1\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED1 ON</button>") ;
                                   client.println("<button class=\"button\" name=\"LED1\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED1 OFF</button><br><br>");
                                 }
                              else 
                                 { 
                                  client.println("<center> <button class=\"button\" name=\"LED1\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED1 ON</button>") ;     
                                  client.println("<button class=\"button\" name=\"LED1\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED1 OFF</button><br><br>");
                                 }  
                                 
                               client.println("<p>LED three is " + LED3STATE + "</p>");
                              if (LED3STATE == "on")
                                 {
                                   client.println("<center> <button class=\"button\" name=\"LED2\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED2 ON</button>") ;
                                   client.println("<button class=\"button\" name=\"LED2\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED2 OFF</button><br><br>");
                                 }
                              else 
                                 { 
                                  client.println("<center> <button class=\"button\" name=\"LED2\" value=\"ON\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#FF0000;color:green;border-style:outset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED2 ON</button>") ;     
                                  client.println("<button class=\"button\" name=\"LED2\" value=\"OFF\" type=\"submit\" style='font-size:16px; text-Shadow:2px 2px 2px green;background-color:#00FF00;color:green;border-style:inset; width:170px;border-radius:15px;border-width: 5px; border-color:#287EC7;'>LED2 OFF</button><br><br>");
                                 }  
      
                              client.println("</fieldset>");   
                              client.println("</center></form></body></html>");
                              client.println();
                              break;
                           } 
                        else 
                           { 
                             current_data_line = "";
                           }
                      }   
                 else if (new_byte != '\r') 
                      { 
                          current_data_line += new_byte; 
                      }   // newbyte if statement ends here
                 }  // client available if statemnet ends here
             }      // while loop ends here
            // Clear the header variable
            header = "";
            // Close the connection
            client.stop();
            Serial.println("Client disconnected.");
            Serial.println("");
         } // outmost if statement ends here
          delay(15);
    }
}
//***************************************************************
void loop()
{
  Serial.println("testing of main loop");
}
//***************************************************************
//***************************************************************
void getpass()
{
  char tempchr[10];
  String tempstr;
  
  WIFI_NAME=NULL;
  tempchr[0]='\0';
  tempstr ="USERID";
  
  //WIFI_NAME=tempstr.toCharArray(tempchr,8);
  //WIFI_NAME=tempstr.c_str();
  //WIFI_NAME=&tempstr[0];
  WIFI_NAME="USERID";
  //strcpy(WIFI_NAME,tempchr);
  
  WIFI_PASSWORD=NULL; 
  tempchr[0]='\0';
  tempstr ="PASSWORD";
  
  //WIFI_PASSWORD=&tempstr[0];
  WIFI_PASSWORD="PASSWORD";
  //WIFI_PASSWORD=tempstr.c_str();
  //strcpy(WIFI_PASSWORD,tempchr);  
}
