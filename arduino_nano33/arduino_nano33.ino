//#include <Arduino_LSM9DS1.h>
#include <Wire.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
//#include <tensorflow/lite/version.h>
#include "model_movement.h"

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 56 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));


const float accelerationThreshold = 0.5; // threshold of significant in G's
const int numSamples = 102;
int samplesRead = 0;
// array to map gesture index to a name
const char* GESTURES[] = {
  "Caminando",
  "Corriendo",
  "Pase_corto_balon",
  "Remate_balon",
  "Saltando"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))
byte cont_acel = 0;

void setup() {
  Serial.begin(115200);  
  while (!Serial);

  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // function that executes whenever data is received from writer

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

}

void loop() {
  delay(1);
}

void receiveEvent(int howMany) {
  String dataString = "";
  float aceleracion = 0.0F;

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a character      
    dataString = dataString + c;  
  }

  aceleracion = dataString.toFloat();
  aceleracion = (aceleracion + 29.0) / 54.0;
  //Pendiente normalizacion
  tflInputTensor->data.f[samplesRead * 3 + cont_acel] = aceleracion ;

  Serial.print(aceleracion);
  Serial.print(";");
  Serial.print(cont_acel);
  Serial.print(";");
  Serial.print(samplesRead);
  Serial.println();

  cont_acel++;      
  if(cont_acel==3){
    cont_acel=0;
  }

    samplesRead++;
    if (samplesRead == numSamples) {
      samplesRead=0;
      Serial.println("Ingresa a inferencia!");

      // Run inferencing
      TfLiteStatus invokeStatus = tflInterpreter->Invoke();

      int pred_index = 0;
      float pred_value = tflOutputTensor->data.f[0];

      // Loop through the output tensor values from the model
      for (int i = 0; i < NUM_GESTURES; i++) {

        Serial.print(GESTURES[i]);
        Serial.print(": ");
        Serial.println(tflOutputTensor->data.f[i], 6);

        if (tflOutputTensor->data.f[i] > pred_value){
          pred_index = i;
          pred_value = tflOutputTensor->data.f[i];
        }          
      } 

      if (pred_value >= 0.8){
        Serial.println(GESTURES[pred_index]);
      }
      if (pred_value < 0.8){
        Serial.println("No supero umbral de predicción");
      }




      

       




    } 
  
}
