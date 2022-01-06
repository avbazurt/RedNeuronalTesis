#pragma once

#include <PZEM004Tv30.h>

#define PZEM_RX_PIN GPIO_NUM_16
#define PZEM_TX_PIN GPIO_NUM_17
#define PZEM_SERIAL Serial2



class MedidorTrifasico
{
private:
    struct ParametrosFase
    {
        float VLN; //VOLTAJE LINEA A NEUTRO [V]
        float VLL; //VOLTAJE LINEA A LINEA [VLL]
        float I;   //CORRIENTE
        float P;    //POTENCIA REACTIVA
        float Q;
        float S;
        float angule;
        float E;
        float f;
        float FP;
    };

    
    void GetSensorMedicion(PZEM004Tv30 fase,ParametrosFase& struct_fase);

    float asin(float c);
    float acos(float c);
    float atan(float c);


public:

    // Constructor
    MedidorTrifasico();

    // Funciones
    void GetMedicionTrifasica();

    // Objetos
    PZEM004Tv30 faseA;
    PZEM004Tv30 faseB;
    PZEM004Tv30 faseC;

    // Variables
    ParametrosFase DatosFaseA;
    ParametrosFase DatosFaseB;
    ParametrosFase DatosFaseC;

    float P3;
    float Q3;
    float S3;
};