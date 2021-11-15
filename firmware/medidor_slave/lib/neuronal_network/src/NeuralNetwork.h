#ifndef __NeuralNetwork__
#define __NeuralNetwork__

#include <stdint.h>

#define len_input 3
#define len_output 1

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
    tflite::MicroMutableOpResolver<15> *resolver;
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
    long int len_new_model_tflite;

    long int indice_new_model;


    //Arreglo donde se almacena el modelo exportado de TensorFLow
    char *model_tflite;

    //Tamaño del buffer a utilizar
    const int kArenaSize = 70000;

    // Funcion que permite cargar el modelo desde memoria SPIFF al arreglo model_tflite
    bool SaveModel();

    bool LoadSPIFFSModel();
    bool LoadDefaultModel();

    // Agregamos entradas
    float *getInputBuffer();
    
    // Predecimos la salida
    float *getOutputBuffer();

};

#endif