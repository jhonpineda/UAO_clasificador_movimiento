//Sensor MPU6050
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
//PROTOCOLO ESP-NOW
#include <esp_now.h>
#include <WiFi.h>

#define SMPLRT_DIV 0x19  //0x19 (HEX) = 25 (DECIMAL) DIRECCION DEL REGISTRO
#define WHO_AM_I_MPU6050 0x75 // Should return 0x68

// Using the GY-521 breakout board, I set ADO to 0 by grounding through a 4k7 resistor
// Seven-bit device address is 110100 for ADO = 0 and 110101 for ADO = 1
#define ADO 0
#if ADO
#define MPU6050_ADDRESS 0x69  // Device address when ADO = 1
#else
#define MPU6050_ADDRESS 0x68  // Device address when ADO = 0
#endif

// instantiates an object of the Adafruit_MPU6050 class
Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp, *mpu_accel, *mpu_gyro;

float x_initial, y_initial, z_initial;
float ax, ay, az;

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xE8, 0x31, 0xCD, 0xD7, 0x12, 0xD4};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float a; 
  float b;
  float c;
  int d;
} struct_message;

// Create a struct_message called myData
struct_message myData;
esp_now_peer_info_t peerInfo;

const float accelerationThreshold = 0; // 2.5 threshold of significant in G's
const int numSamples = 150;

int samplesRead = numSamples;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("\r\nLast Packet Send Status:\t");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.print("");
}

void setup(void) {
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  while (!Serial) {
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
  }
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu_accel = mpu.getAccelerometerSensor();
  mpu_accel->printSensorDetails();
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  writeByte(MPU6050_ADDRESS, SMPLRT_DIV, 0x19);  // Use a 300 Hz sample rate 
  
  // print the header document csv
  Serial.println("aX,aY,aZ");
  delay(100);
  calibrate_mpu();
}

void loop() { 
  read_data();
}

void read_data() {

  //while (samplesRead < numSamples) {
    // read the acceleration data
    sensors_event_t accel;
    mpu_accel->getEvent(&accel);

    ax = accel.acceleration.x - x_initial;
    ay = accel.acceleration.y - y_initial;
    az = accel.acceleration.z - z_initial;
    
    // print the data in CSV format (acceleration is measured in m/s^2)
    Serial.print(ax, 3);
    Serial.print(',');
    Serial.print(ay, 3);
    Serial.print(',');
    Serial.print(az, 3);
    Serial.print(',');
    Serial.print(samplesRead);
    //Serial.println();

    // Set values to send
    myData.a = ax;
    myData.b = ay;
    myData.c = az;
    myData.d = samplesRead;    
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));   
    if (result == ESP_OK) {
      Serial.print(',');
      Serial.print("Sent data with success");
      Serial.println();
    }
    else {
      Serial.print(',');
      Serial.print("Error sending the data");
      Serial.println();
    }

    samplesRead++;
  //}
}

void calibrate_mpu(){
  float totX, totY, totZ;
  sensors_event_t a, g, temp;  
  
  for (int i = 0; i < 100; i++) {
    mpu.getEvent(&a, &g, &temp);
    totX = totX + a.acceleration.x;
    totY = totY + a.acceleration.y;
    totZ = totZ + a.acceleration.z;
  }
  x_initial = totX / 100;
  y_initial = totY / 100;
  z_initial = totZ / 100;
  Serial.println("Calibrated");
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire.beginTransmission(address);  // Initialize the Tx buffer
	Wire.write(subAddress);           // Put slave register address in Tx buffer
	Wire.write(data);                 // Put data in Tx buffer
	Wire.endTransmission();           // Send the Tx buffer
}
uint8_t readByte(uint8_t address, uint8_t subAddress)
{
	uint8_t data; // `data` will store the register data	 
	Wire.beginTransmission(address);         // Initialize the Tx buffer
	Wire.write(subAddress);	                 // Put slave register address in Tx buffer
	Wire.endTransmission(false);             // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(address, (uint8_t) 1);  // Read one byte from slave register address 
	data = Wire.read();                      // Fill Rx buffer with result
	return data;                             // Return data read from slave register
}

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest)
{  
	Wire.beginTransmission(address);   // Initialize the Tx buffer
	Wire.write(subAddress);            // Put slave register address in Tx buffer
	Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
	uint8_t i = 0;
        Wire.requestFrom(address, count);  // Read bytes from slave register address 
	while (Wire.available()) {
        dest[i++] = Wire.read(); }         // Put read results in the Rx buffer
}