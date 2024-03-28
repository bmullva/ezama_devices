//BOARD IS AI THINKER ESP32-CAM

#include <Ezama.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"



// 1 INITIALIZE DEVICE PARTICULAR CONSTANTS & VARIABLES
String type = "still cam";
String ver = "7.0";
String serverPath = "/upload-image/" + String(device_id);     // The default serverPath should be upload.php
const int serverPort = 1880;
String serverName {};

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


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
  state_json["action"] = "submits photo";
  state_json["descript"] = descript;
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
  sendPhoto();
}


// 3 REPORT ID: "mqtt_pub -h XXX.XXX.XXX.XXX -m ids -t broadcast"
void publish_ids_json() {
  String output;
  DynamicJsonDocument state_json(1024);
  char sj[1024];
  String topic = "broadcast";
  state_json["device_id"] = device_id;
  state_json["type"] = type;
  state_json["cat"] = "sensor";  // sensor, controller, actuator
  state_json["ex"] = "this device sends a picture when pinged";
  serializeJson(state_json, output);
  output.toCharArray(sj, 1024);
  client.publish(topic.c_str(), sj);
}


// 4 RECEIVE CONTROLS (to this exact device, from callback)
void receive_controls_json(String messageTemp) {
  if(messageTemp == "restart") {ESP.restart();}
}


// 5 SEND CONTROLS SEND CONTROLS (publish_controls only if controller module)
void publish_controls_json(String pin_name, String pin_msg) {

}


//6 SETUP (pins)
void setup() { 
  ezama_setup();  //in ezama.h

  serverName = mqtt_ip; 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //config.frame_size = FRAMESIZE_QQVGA; // 160x120 pixels
  //config.frame_size = FRAMESIZE_QCIF; // 176x144 pixels
  //config.frame_size = FRAMESIZE_HQVGA; // 240x176 pixels
  //config.frame_size = FRAMESIZE_240X240; // 240x240 pixels
  config.frame_size = FRAMESIZE_QVGA; // 320x240 pixels
  //config.frame_size = FRAMESIZE_CIF; // 400x296 pixels
  //config.frame_size = FRAMESIZE_HVGA; // 480x320 pixels
  //config.frame_size = FRAMESIZE_VGA; // 640x480 pixels
  //config.frame_size = FRAMESIZE_SVGA; // 800x600 pixels
  //config.frame_size = FRAMESIZE_XGA; // 1024x768 pixels
  config.jpeg_quality = 12;  //0-63 lower number means higher quality
  config.fb_count = 2;
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    delay(1000);
    ESP.restart();
  }
}


//7 MAIN LOOP
void loop() {
  ezama_loop();  //in ezama.h
  
  }


void sendPhoto() {
  WiFiClient client2;
  WiFi.mode(WIFI_STA);
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    delay(1000);
    ESP.restart();
  }

  if (client2.connect(serverName.c_str(), serverPort)) { 
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client2.println("POST " + serverPath + " HTTP/1.1");
    client2.println("Host: " + serverName);
    client2.println("Content-Length: " + String(totalLen));
    client2.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    client2.println();
    client2.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client2.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client2.write(fbBuf, remainder);
      }
    }   
    client2.print(tail);

    esp_camera_fb_return(fb);
  } else {
    esp_camera_fb_return(fb);
    client2.stop();
    client2.connect(serverName.c_str(), serverPort);
    return;
  }
}
