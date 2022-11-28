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

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    float a;
    float b;
    float c;
    int d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  //Imprimir por la terminal serial
  Serial.print(myData.d);
  Serial.print(',');
  Serial.print(myData.a);
  Serial.print(',');  
  Serial.print(myData.b);
  Serial.print(',');  
  Serial.print(myData.c);
  Serial.println();

  //delay(2000); // Pause for 2 seconds
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  Serial.println("accX,accY,accZ");
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }	 	  
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}