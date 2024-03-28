/*
  Sketch to demonstrate using the print class with smooth fonts,
  the Smooth fonts are stored in a FLASH program memory array.

  Sketch is written for a 240 x 320 display

  New font files in the .vlw format can be created using the Processing
  sketch in the library Tools folder. The Processing sketch can convert
  TrueType fonts in *.ttf or *.otf files.

  The library supports 16 bit unicode characters:
  https://en.wikipedia.org/wiki/Unicode_font

  The characters supported are in the in the Basic Multilingual Plane:
  https://en.wikipedia.org/wiki/Plane_(Unicode)#Basic_Multilingual_Plane

  Make sure all the display driver and pin connections are correct by
  editing the User_Setup.h file in the TFT_eSPI library folder.
*/

//  The font is stored in an array within a sketch tab.

//  A processing sketch to create new fonts can be found in the Tools folder of TFT_eSPI
//  https://github.com/Bodmer/TFT_eSPI/tree/master/Tools/Create_Smooth_Font/Create_font

// Graphics and font library
#include <TFT_eSPI.h>
#include <SPI.h>
//#include "Final_Frontier_28.h"

//#include <Adafruit_GFX.h>    // Core graphics library
//#include <FreeMono12pt7b.h>  
//#include <FreeSansBoldOblique12pt7b.h>
//#include <FreeMono18pt7b.h>
//#include <FreeSansBoldOblique18pt7b.h>
//#include <FreeMono24pt7b.h>
//#include <FreeSansBoldOblique24pt7b.h>
//#include <FreeMono9pt7b.h>    
//#include <FreeSansBoldOblique9pt7b.h>
//#include <FreeMonoBold12pt7b.h>
//#include <FreeSansOblique12pt7b.h>
//#include <FreeMonoBold18pt7b.h>
//#include <FreeSansOblique18pt7b.h>
//#include <FreeMonoBold24pt7b.h>
//#include <FreeSansOblique24pt7b.h>
//#include <FreeMonoBold9pt7b.h>
//#include <FreeSansOblique9pt7b.h>
//#include <FreeMonoBoldOblique12pt7b.h>
//#include <FreeSerif12pt7b.h>
//#include <FreeMonoBoldOblique18pt7b.h>
//#include <FreeSerif18pt7b.h>
//#include <FreeMonoBoldOblique24pt7b.h>
//#include <FreeSerif24pt7b.h>
//#include <FreeMonoBoldOblique9pt7b.h>
//#include <FreeSerif9pt7b.h>
//#include <FreeMonoOblique12pt7b.h>
//#include <FreeSerifBold12pt7b.h>
//#include <FreeMonoOblique18pt7b.h>
//#include <FreeSerifBold18pt7b.h>
//#include <FreeMonoOblique24pt7b.h>
//#include <FreeSerifBold24pt7b.h>
//#include <FreeMonoOblique9pt7b.h>
//#include <FreeSerifBold9pt7b.h>
//#include <FreeSans12pt7b.h>
//#include <FreeSerifBoldItalic12pt7b.h>
//#include <FreeSans18pt7b.h>
//#include <FreeSerifBoldItalic18pt7b.h>
//#include <FreeSans24pt7b.h>
//#include <FreeSerifBoldItalic24pt7b.h>
//#include <FreeSans9pt7b.h>
//#include <FreeSerifBoldItalic9pt7b.h>
//#include <FreeSansBold12pt7b.h>
//#include <FreeSerifItalic12pt7b.h>
//#include <FreeSansBold18pt7b.h>
//#include <FreeSerifItalic18pt7b.h>
//#include <FreeSansBold24pt7b.h>
//#include <FreeSerifItalic24pt7b.h>
//#include <FreeSansBold9pt7b.h>
//#include <FreeSerifItalic9pt7b.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library
String ip_addr = "192.168.222.222";
// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200); // Used for messages

  tft.init();
  tft.setRotation(1);
  tft.setTextWrap(true, true);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);  
  //tft.setFont(&FreeMonoBoldOblique12pt7b);
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
void loop() {
  //tft.loadFont();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.println("IP:  ");
  tft.setTextSize(2);
  tft.println(ip_addr);
  tft.println();

  tft.setTextSize(2);
  if (true) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("1 ");

  if (true) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("2 ");
  
  if (false) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("3 ");
  
  if (true) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("4 ");
  
  if (false) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.print("5 ");
  
  if (true) {tft.setTextColor(TFT_GREEN, TFT_BLACK);}
  else {tft.setTextColor(TFT_RED, TFT_BLACK);}
  tft.println("6 ");
  delay(4000);
  

  //THIS IS FOR ADAFRUIT
  //tft.setTextColor(TFT_RED, TFT_BLACK);
  //tft.fillScreen(TFT_BLACK);
  //tft.setTextSize(2);
  //tft.println("This is Adafruit, text 2.");
  //delay(5000);

  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.fillScreen(TFT_BLACK);
  //tft.setTextSize(4);
  //tft.println("This is Adafruit, text 2.");
  //delay(5000);

  // Unload the font to recover used RAM
  //tft.unloadFont();  // not sure why this is used

}
