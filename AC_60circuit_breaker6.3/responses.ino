//*************************************************
//***  FROM CALLBACK, CALLBACK, LOOP, CALLBACK  ***
//*************************************************

void publish_reporting() {   

  slg1 = (digitalRead(25) == LOW) ? "on" : "off";
  slg2 = (digitalRead(22) == LOW) ? "on" : "off";
  slg3 = (digitalRead(21) == LOW) ? "on" : "off";
  slg4 = (digitalRead(19) == LOW) ? "on" : "off";
  slg5 = (digitalRead(17) == LOW) ? "on" : "off";
  slg6 = (digitalRead(16) == LOW) ? "on" : "off";
  
  // THIS WILL BE SENT TO THE SCREEN WHEN IT IS WRITTEN
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.println("IP:  ");
  tft.println(WiFi.localIP());
  tft.println();

  if (slg1 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("1 ");

  if (slg2 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("2 ");
  
  if (slg3 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("3 ");
  
  if (slg4 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("4 ");
  
  if (slg5 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("5 ");
  
  if (slg6 == "on") {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.println("6 ");
}


void receive_controls(String messageTemp) {  // NOT SURE WHAT THIS LOOKS LIKE, BUT WILL RECEIVE BUTTON CLICKS FROM WEBSITE


}
