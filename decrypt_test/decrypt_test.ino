#include <iostream>


//using namespace std;

String cipher(String text, int key) {
  String encrypted;
  for (char c : text) {
    if (c >= ' ' && c <= '~') {
      // Shift the character by the key, wrapping around if necessary.
      int encrypted_char_code = (c - 35 + key) % 92 + 35;
      encrypted += char(encrypted_char_code);
    } else {
      // If the character is outside the printable ASCII range, leave it unchanged.
      encrypted += c;
    }
  }
  return encrypted;
}

String x = "0000003G";
char keyStr = x.charAt(x.length() - 1);
int key = isdigit(keyStr) ? keyStr - '0' : tolower(keyStr) - 'a' + 10;
String dataStr = "D@HFGID@D@";


void setup() {
  Serial.begin(115200);
  
  String encrypted_data = cipher(dataStr, -key);

  Serial.println();
  Serial.print("Encrypted: ");
  Serial.println(encrypted_data);

  String decrypted_data = cipher(encrypted_data, key);

  Serial.print("Decrypted: ");
  Serial.println(decrypted_data);
}

void loop() {
  // Your loop code here
}
