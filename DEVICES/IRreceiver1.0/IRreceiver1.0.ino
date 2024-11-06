#include <Arduino.h>
#include <Ezama12.h>

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 * This must be done before the #include <IRremote.hpp>
 */
//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
//#define DECODE_LG
//#define DECODE_NEC          // Includes Apple and Onkyo. To enable all protocols , just comment/disable this line.
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6

//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER
//#define DECODE_FAST

//#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
//#define DECODE_HASH         // special decoder for all protocols

//#define DECODE_BEO          // This protocol must always be enabled manually, i.e. it is NOT enabled if no protocol is defined. It prevents decoding of SONY!

//#define DEBUG               // Activate this for lots of lovely debug output from the decoders.

//#define RAW_BUFFER_LENGTH  750 // For air condition remotes it requires 750. Default is 200.

/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type_ = "IRreceiver";
String ver = "1.0";


// 2 REPORT (SENT EVERY 6 SECONDS)
void publish_reporting_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "reporting";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["ver"] = ver;
  state_json["IP"] = WiFi.localIP();
  //state_json["vG"]        = "amp,0,20";
  //state_json["vL"]        = "1,12,onOff;1,12,lux";
  //state_json["pL"]        = "1,12,";
  state_json["pS"]        = "1,1,";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
//Reserve


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String topic, String messageTemp) {

}


// 5 PUBLISH AND SEND CONTROLS (publish_controls only if controller module)
void publish_controls(String switch_num, String pin_msg) {

}


//6 SETUP (pins)
void specific_connect() {
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    ezama_setup();
    
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
}


//7 MAIN LOOP
//These are the messages that are being sent: "hold", "click", "click-hold", "dbl-click", "release"

void loop() {
  if (IrReceiver.decode()) {
    // Check if this is not a repeat signal
    if (!IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      uint8_t command = IrReceiver.decodedIRData.command; // Store command in a variable

      // Check if command equals 0x16
      if (command == 0x16) {
        Serial.println("star");
      }
      // Check if command equals 0xD
      else if (command == 0xD) {
        Serial.println("hash");
      } else {
        // Optional: Print other commands
        //Serial.print("Command: 0x");
        Serial.println(command, HEX);
      }
    }

    // Resume receiver to get the next signal
    IrReceiver.resume();
  }
}
