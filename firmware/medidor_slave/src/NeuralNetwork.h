#ifndef __NeuralNetwork__
#define __NeuralNetwork__

#include <stdint.h>

namespace tflite
{
    template <unsigned int tOpCount>
    class MicroMutableOpResolver;
    class ErrorReporter;
    class Model;
    class MicroInterpreter;
} // namespace tflite

struct TfLiteTensor;

class NeuralNetwork
{
private:
    tflite::MicroMutableOpResolver<10> *resolver;
    tflite::ErrorReporter *error_reporter;
    const tflite::Model *model;
    tflite::MicroInterpreter *interpreter;
    TfLiteTensor *input;
    TfLiteTensor *output;
    uint8_t *tensor_arena;

public:
    //Inicializador
    NeuralNetwork();
    char *new_model_tflite;

    //Arreglo donde se almacena el modelo exportado de TensorFLow
    char *model_tflite;

    //Tamaño del buffer a utilizar
    const int kArenaSize = 50000;

    // Funcion que permite cargar el modelo desde memoria SPIFF al arreglo model_tflite
    bool LoadModel();

    // Agregamos entradas
    float *getInputBuffer();
    
    // Predecimos la salida
    float predict();
};

#endif