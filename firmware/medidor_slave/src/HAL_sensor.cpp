#include "HAL_sensor.h"

MedidorTrifasico::MedidorTrifasico() : faseA(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x05),
                                       faseB(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x20),
                                       faseC(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x05)
{
    // pzem.setCalibration();
    DatosFaseA = {0, 0, 0, 0, 0, 0, 0 ,0 ,0};
    DatosFaseB = {0, 0, 0, 0, 0, 0, 0 ,0 ,0};
    DatosFaseC = {0, 0, 0, 0, 0, 0, 0 ,0 ,0};

    //Inicializo
    P3 = 0;
    Q3 = 0;
    S3 = 0;
}

float MedidorTrifasico::asin(float c)
// Solucion utilizada de https://www.instructables.com/Arduino-Trigonometric-Inverse-Functions/
{
    float out;
    out = ((c + (c * c * c) / 6 + (3 * c * c * c * c * c) / 40 + (5 * c * c * c * c * c * c * c) / 112 +
            (35 * c * c * c * c * c * c * c * c * c) / 1152 + (c * c * c * c * c * c * c * c * c * c * c * 0.022) +
            (c * c * c * c * c * c * c * c * c * c * c * c * c * .0173) + (c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * .0139) +
            (c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * 0.0115) + (c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * c * 0.01)));
    // asin
    if (c >= .96 && c < .97)
    {
        out = 1.287 + (3.82 * (c - .96));
    }
    if (c >= .97 && c < .98)
    {
        out = (1.325 + 4.5 * (c - .97));
    } // arcsin
    if (c >= .98 && c < .99)
    {
        out = (1.37 + 6 * (c - .98));
    }
    if (c >= .99 && c <= 1)
    {
        out = (1.43 + 14 * (c - .99));
    }
    return out;
}

float MedidorTrifasico::acos(float c)
// Solucion utilizada de https://www.instructables.com/Arduino-Trigonometric-Inverse-Functions/
{
    float out;
    out = asin(sqrt(1 - c * c));
    return out;
}

float MedidorTrifasico::atan(float c)
// Solucion utilizada de https://www.instructables.com/Arduino-Trigonometric-Inverse-Functions/
{
    float out;
    out = asin(c / (sqrt(1 + c * c)));
    return out;
}

void MedidorTrifasico::GetSensorMedicion(PZEM004Tv30 fase, ParametrosFase& struct_fase)
{

    // Obtener Medicion Factor Potencia
    struct_fase.FP = fase.pf();
    if (isnan(struct_fase.FP))
    {
        struct_fase.FP = -1;
    }
    else{
        struct_fase.angule = acos(struct_fase.FP);
    }

    // Obtener Medicion Voltaje
    struct_fase.VLN = fase.voltage();
    if (isnan(struct_fase.VLN))
    {
        struct_fase.VLN = -1;
    }

    // Obtener Medicion Corriente
    struct_fase.I = fase.current();
    if (isnan(struct_fase.I))
    {
        struct_fase.I = -1;
    }

    // Obtener Medicion Potencia
    struct_fase.P = fase.power();
    if (isnan(struct_fase.P))
    {
        struct_fase.P = -1;
    }
    else{
        struct_fase.P = struct_fase.P/1000;
        struct_fase.Q = struct_fase.P*sin(struct_fase.angule);
        struct_fase.S = struct_fase.VLN*struct_fase.I;
    }

    // Obtener Medicion Energia
    struct_fase.E = fase.energy();
    if (isnan(struct_fase.E))
    {
        struct_fase.E = -1;
    }

    // Obtener Medicion Frecuencia
    struct_fase.f = fase.frequency();
    if (isnan(struct_fase.f))
    {
        struct_fase.f = -1;
    }

}

void MedidorTrifasico::GetMedicionTrifasica()
{
    GetSensorMedicion(faseA, DatosFaseA);
    GetSensorMedicion(faseB, DatosFaseB);
    GetSensorMedicion(faseC, DatosFaseC);

    P3 = (DatosFaseA.P +  DatosFaseB.P + DatosFaseC.P);
    Q3 = (DatosFaseA.Q +  DatosFaseB.Q + DatosFaseC.Q);
    S3 = (DatosFaseA.S +  DatosFaseB.S + DatosFaseC.S)/1000;
}