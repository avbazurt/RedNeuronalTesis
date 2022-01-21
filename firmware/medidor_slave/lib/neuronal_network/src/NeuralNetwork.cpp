#include "NeuralNetwork.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "model_data.h"

#include <Preferences.h>
#include "SPIFFS.h"


#define file_model "/model.txt"

NeuralNetwork::NeuralNetwork()
{
    error_reporter = new tflite::MicroErrorReporter();

    LoadModel();

    model = tflite::GetModel(model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Model provided is schema version %d not equal to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
    // This pulls in the operators implementations we need
    resolver = new tflite::MicroMutableOpResolver<15>();
    resolver->AddFullyConnected();
    resolver->AddMul();
    resolver->AddAdd();
    resolver->AddLogistic();
    resolver->AddReshape();
    resolver->AddQuantize();
    resolver->AddDequantize();
    resolver->AddConv2D();
    resolver->AddConcatenation();
    resolver->AddSoftmax();
    resolver->AddMaxPool2D();

    tensor_arena = (uint8_t *)malloc(kArenaSize);
    if (!tensor_arena)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Could not allocate arena");
        return;
    }

    // Build an interpreter to run the model with.
    interpreter = new tflite::MicroInterpreter(
        model, *resolver, tensor_arena, kArenaSize, error_reporter);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    size_t used_bytes = interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(error_reporter, "Used bytes %d\n", used_bytes);

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);
}

bool NeuralNetwork::SaveModel()
{

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }

    SPIFFS.remove(file_model);

    File file = SPIFFS.open(file_model, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return false;
    }

    //Guardamos el nuevo tamaño del modelo
    file.println(len_new_model_tflite);

    int valor;
    for (int indice = 0; indice < len_new_model_tflite; indice++)
    {
        char msg[10];
        valor = new_model_tflite[indice];
        sprintf(msg, "0x%02X\n", valor);
        file.print(String(msg));
    }

    file.close();
    return true;
}

bool NeuralNetwork::LoadModel()
{
    if (!SPIFFS.begin(true))
    {
        log_e("An Error has occurred while mounting SPIFFS");
        return false;
    }

    if (!SPIFFS.exists(file_model)){
        log_e("No se registra un modelo TFlite en memoria SPIFFS, se procede a utilizar un modelo generico");

        log_i("Tamaño Buffer a crear: %d", converted_model_tflite_len);
        model_tflite = new char[converted_model_tflite_len];

        for (int indice = 0; indice<converted_model_tflite_len; indice++){
            model_tflite[indice] = converted_model_tflite[indice];
        }
        return true;
    }


    File file = SPIFFS.open(file_model);
    if (!file)
    {
        log_e("Failed to open file for reading");
        return false;
    }

    int m = 0;

    String linea;

    char fila[20];
    char digito;

    int i;

    while (file.available())
    {
        i = 0;
        digito = file.read();

        while (digito != '\n' and file.available())
        {
            fila[i] = digito;
            i++;
            digito = file.read();
        }

        if (m == 0)
        {
            linea = String(fila);
            int len_model = linea.toInt();

            log_i("Tamaño Buffer a crear: %d", len_model);
            model_tflite = new char[len_model];
        }
        else
        {
            linea = String(fila);
            linea = linea.substring(2, 4);

            char msg[10];
            linea.toCharArray(msg, 10);

            int myInt = (long)strtol(msg, 0, 16);

            model_tflite[m - 1] = myInt;
        }
        m++;
    }
    file.close();
    log_i("Modelo Cargado");
    return true;
}

float *NeuralNetwork::getInputBuffer()
{
    return input->data.f;
}

float *NeuralNetwork::getOutputBuffer()
{
    interpreter->Invoke();
    return output->data.f;
}
