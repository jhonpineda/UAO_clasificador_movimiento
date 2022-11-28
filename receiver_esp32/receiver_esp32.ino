//Get MAC Addres ESP32 receiver
//#include "WiFi.h" 
//void setup(){
//  Serial.begin(115200);
//  WiFi.mode(WIFI_MODE_STA);
//  Serial.println(WiFi.macAddress());
//} 
//void loop(){
//}

#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    float a;
    float b;
    float c;
    byte d;
} struct_message;

char x[8];

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //Serial.print("ax:");
  Serial.println(myData.a);

  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(dtostrf(myData.a,5,3,x));
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(dtostrf(myData.b,5,3,x));
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(dtostrf(myData.c,5,3,x));
  Wire.endTransmission();    // stop transmitting

  //Wire.beginTransmission(8); // transmit to device #8
  //Wire.write("@");
  //Wire.endTransmission();    // stop transmitting
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  
  Wire.begin();
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }	 	
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}