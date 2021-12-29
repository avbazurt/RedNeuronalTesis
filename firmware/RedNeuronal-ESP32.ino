#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "data.h"

namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 15 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
}



void setup() {
  Serial.begin(115200);

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;


  model = tflite::GetModel(mi_data);

  static tflite::ops::micro::AllOpsResolver resolver;

  // Cree un intérprete para ejecutar el modelo.
  static tflite::MicroInterpreter static_interpreter( model,
      resolver,
      tensor_arena,
      kTensorArenaSize,
      error_reporter);
  interpreter = &static_interpreter;

  // Asignar memoria  para los tensores del modelo.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();

  // Obtenga punteros a los tensores de entrada y salida del modelo.
  input = interpreter->input(0);
  output = interpreter->output(0);


}

void loop() {

  int x1 = 5.01 * random(1, 100) / 100;
  int x2 = 5.01 * random(1, 100) / 100;

  input->data.f[0] = x1;
  input->data.f[1] = x2;
  TfLiteStatus invoke_status = interpreter->Invoke();

  float out = output->data.f[0];
  Serial.println("|-----------------------TensorFlowLite-ESP32-----------------------|");
  Serial.print("x1: ");
  Serial.println(x1);
  Serial.print("x2: ");
  Serial.println(x2);
  Serial.print("Predicion: ");
  Serial.println(out);
  Serial.println("|------------------------------------------------|\n");

  delay(2000);

}

//by sandro ormeño
