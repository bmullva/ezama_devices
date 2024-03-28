//*********************************
//***  FROM SETUP() AND LOOP()  ***
//*********************************

void initialize_pins() {
  //pins used by TFT, found in User_Setup.h
  #define TFT_MOSI 23
  #define TFT_SCLK 18
  #define TFT_CS    5
  #define TFT_DC    2
  #define TFT_RST   4
  
  pinMode(25, OUTPUT); // SLG 1
  pinMode(22, OUTPUT); // SLG 2
  pinMode(21, OUTPUT); // SLG 3
  pinMode(19, OUTPUT); // SLG 4
  pinMode(17, OUTPUT); // SLG 5
  pinMode(16, OUTPUT); // SLG 6
  
  pinMode(36, INPUT); // VOG 1
  pinMode(39, INPUT); // GOV 2
  pinMode(34, INPUT); // GOV 3
  pinMode(35, INPUT); // VOG 4
  pinMode(32, INPUT); // VOG 5
  pinMode(33, INPUT); // VOG 6
  
  digitalWrite(25, LOW); // SLG 1
  digitalWrite(22, LOW); // SLG 2
  digitalWrite(21, LOW); // SLG 3
  digitalWrite(19, LOW); // SLG 4
  digitalWrite(17, LOW); // SLG 5
  digitalWrite(16, LOW); // SLG 6

}


void send_messages() {

}
