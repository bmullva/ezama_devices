#include <Ezama11.h>  // For ESP-01S
#include <Filters.h>


// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "Tri DB";
String ver = "11";

int dim_amt (int dim) {
  return 100 - dim;
}


//VIRTUAL:
//lux 1-100
//onOff 0, 1
//RGB 0-255
// onOff_array: onOff 0, 1: {All};
// lux_array:  lux 0-100 {All};
// RGB_array:  {Red, Green, Blue};

int onOff_array[]  = {0};
int lux_array[]    = {0};
int RGB_array[]   = {0, 0, 0};
 

//PHYSICAL:
//device_id/N N=1-4 should accept "click", "on", "off", "dim", "brighten", NOT: "heat", "cool"
// +1 -1 or 0: lt_array:  lux, Red, Green, Blue
int lt_array []    = {0, 0, 0, 0};



// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"]      = type_;
  state_json["ver"]       = ver;
  state_json["IP"]        = WiFi.localIP();
  //state_json["vG"]        = "amp,0,20";
  state_json["vL"]        = "0,0,onOff;0,0,lux;1,3,RGB";
  state_json["pL"]        = "0,0,";
  //state_json["pS"]        = "1,4,onOff";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  Serial.print("Returning State json: ");
  Serial.print(topic);
  Serial.println(sj);
  
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
// Reserved


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String msg) {
  
  //VIRTUAL MSGS: 
  // anything goes to device_id/onOffN, 1 ("click")
  // numbers go to device device_id/luxN, 5-100
  // device_id/tempN 0-255

  for (int i = 0; i<=0; i++) {        
    if (topic == String(device_id) + "/" + String("onOff") + String(i)) {
      onOff_array[i] = 1-onOff_array[i];  // any msg will switch between 1 and 0
    }
  }

  for (int i = 0; i<=0; i++) {        
    if (topic == String(device_id) + "/" + "lux" + String(i)) {
      lux_array[i] = msg.toInt();  // msg to this topic should be value between 0-100
    }
  }
    
  for (int i = 0; i<=2; i++) {        
    if (topic == String(device_id) + "/" + "RGB" + String(i)) {
      RGB_array[i] = msg.toInt();  // msg to this topic should be value between 0-255
    }
  }

  //PHYSICAL MESSAGES
  //device_id/N N=1 should accept "on", "off", "dim", "brighten", NOT "heat", "cool", "release"
  //lm1Lux

  for (int i = 0; i<=0; i++) {        
    if (topic == String(device_id) + "/" + String(i)) {
        // msg to these topics should be "on", "off", "dim", "brighten", NOT "heat", "cool", "release"
               
        if (msg == "on") {
          onOff_array[i] = 1;
        }

        if (msg == "off") {
          onOff_array[i] = 0;
        }
        
        if (msg == "dim") {
          lt_array[i] = 1;
        }

        if (msg == "brighten") {
          lt_array[i] = -1;
        }

        // if (msg == "heat") {
        //  lt_array[i+4] = 1;
        //}
        
        //if (msg == "cool") {
        //  lt_array[i+4] = -1;
        //}

        if (msg == "release") {
          lt_array[i] = 0;
          lt_array[i+4] = 0;
        }
    }
  }


  //NAC DOES NOT EXIST on this Light Hub

  analogWrite(0, dim_amt(lux_array[0]) *     RGB_array[0]   * onOff_array[0] /100 );    //Red
  analogWrite(1, dim_amt(lux_array[0]) *     RGB_array[1]   * onOff_array[0] /100 );    //Green
  analogWrite(2, dim_amt(lux_array[0]) *     RGB_array[2]   * onOff_array[0] /100 );    //Blue
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
  String topic {};
  
  //topic = String(device_id)+"/"+String("amp");
  //client.subscribe(topic.c_str());
  //if (!client.subscribe(topic.c_str())) {
  //      Serial.println("Failed to subscribe to topic: " + topic);
  //}

  for (int i = 0; i<=0; i++) {        
    topic = String(device_id)+"/"+String(i);
    client.subscribe(topic.c_str());
    }
  
  for (int i = 0; i <= 0; i++) {
    topic = String(device_id) + "/" + String("onOff") + String(i);
    client.subscribe(topic.c_str());
  }

  for (int i = 0; i<=0; i++) {        
    client.subscribe((String(device_id)+"/"+String("lux")+String(i)).c_str());
    }
    
  for (int i = 0; i<=2; i++) {        
    client.subscribe((String(device_id)+"/"+String("RGB")+String(i)).c_str());
    }
}

void setup() { 
  Serial.begin(115200);
  ezama_setup();  //in ezama.h
  
  //pinMode(A0, INPUT);
  pinMode(0, OUTPUT);  // RED
  pinMode(1, OUTPUT);  // GREEN
  pinMode(2, OUTPUT);  // BLUE

  digitalWrite(0, HIGH);    // RED
  digitalWrite(1, HIGH);    // GREEN
  digitalWrite(2, HIGH);    // BLUE

  delay(2000);

  digitalWrite(0, LOW);    // RED
  digitalWrite(1, LOW);    // GREEN
  digitalWrite(2, LOW);    // BLUE

  specific_connect();
}


//7 MAIN LOOP
// -1,0,+1 to manage the analog writing in the lt_array: lm1Lux, lm2Lux, lm3Lux, lm4Lux, lm1Temp, lm2Temp, lm3Temp, lm4Temp

void loop() {
  ezama_loop();  //in ezama.h
  
  for(int i=0;i<=0;i++) {
    if(lt_array[i] == 1){   // like increasing the dim slider
      Serial.print("lt_array[i]=1 and i is:");
      Serial.println(i);
      if (lux_array[i] >= 0 && lux_array[i] < 94) {
        lux_array[i] += 1;
        if (i == 0) {
          analogWrite(0,  dim_amt(lux_array[0]) *   RGB_array[0]   * onOff_array[0] /100 );    //RED
          analogWrite(1,  dim_amt(lux_array[0]) *   RGB_array[1]   * onOff_array[0] /100 );    //GREEN
          analogWrite(2,  dim_amt(lux_array[0]) *   RGB_array[2]   * onOff_array[0] /100 );    //BLUE
        } 
        Serial.println("I am dimming in the 1-4 main loop");
      }
    }
    if(lt_array[i] == -1){  // like decreasing the dim slider
      Serial.print("lt_array [i]=-1 and i is:");
      Serial.println(i);
      if (lux_array[i] > 5 && lux_array[i] <= 100) {
        lux_array[i] -= 1;
        if (i == 0) {
          analogWrite(0,  dim_amt(lux_array[0]) *   RGB_array[0]   * onOff_array[0] /100 );    //RED
          analogWrite(1,  dim_amt(lux_array[0]) *   RGB_array[1]   * onOff_array[0] /100 );    //GREEN
          analogWrite(2,  dim_amt(lux_array[0]) *   RGB_array[2]   * onOff_array[0] /100 );    //BLUE
        } 
        Serial.println("I am brightening in the 1-4 main loop");
      }
    }
  }

  delay(50);
}
