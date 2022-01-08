#include "HAL_NextionUI.h"
#include "HAL_config.h"

// Pausa global de interacción, para flasheo de Nextion
static bool _globalPause = false;
static bool _FlashModel = false;

int page = 0;

// PANTALLA TRIANGULO POTENCIA
#define NEXTION_PAGE_PQS 0
static NexPage page_TrianglePower(NEXTION_PAGE_PQS, NEXTION_PAGE_PQS, "page0");
static NexButton back_TP(NEXTION_PAGE_PQS, 5, "b0");
static NexButton continue_TP(NEXTION_PAGE_PQS, 6, "b1");
static NexText text_P3(NEXTION_PAGE_PQS, 2, "t0");
static NexText text_Q3(NEXTION_PAGE_PQS, 3, "t1");
static NexText text_S3(NEXTION_PAGE_PQS, 4, "t2");
static NexButton opcion_3P(NEXTION_PAGE_PQS, 7, "b2");
static NexButton opcion_3Q(NEXTION_PAGE_PQS, 8, "b3");
static NexButton opcion_3S(NEXTION_PAGE_PQS, 9, "b4");

// PANTALLA VOLTAJE LINEA NEUTRO
#define NEXTION_PAGE_VLN 1
static NexPage page_VoltajeNeutro(NEXTION_PAGE_VLN, NEXTION_PAGE_VLN, "page4");
static NexButton back_VN(NEXTION_PAGE_VLN, 3, "b0");
static NexButton continue_VN(NEXTION_PAGE_VLN, 2, "b1");
static NexText text_VLN1(NEXTION_PAGE_VLN, 4, "t0");
static NexText text_VLN2(NEXTION_PAGE_VLN, 5, "t1");
static NexText text_VLN3(NEXTION_PAGE_VLN, 6, "t2");

// PANTALLA CORRIENTE LINEA
#define NEXTION_PAGE_ILN 2
static NexPage page_Corriente(NEXTION_PAGE_ILN, NEXTION_PAGE_ILN, "page5");
static NexButton back_I(NEXTION_PAGE_ILN, 2, "b0");
static NexButton continue_I(NEXTION_PAGE_ILN, 3, "b1");
static NexText text_I1(NEXTION_PAGE_ILN, 4, "t0");
static NexText text_I2(NEXTION_PAGE_ILN, 5, "t1");
static NexText text_I3(NEXTION_PAGE_ILN, 6, "t2");

// PANTALLA CORRIENTE FP
#define NEXTION_PAGE_FP 3
static NexPage page_FP(NEXTION_PAGE_FP, NEXTION_PAGE_FP, "page6");
static NexButton back_FP(NEXTION_PAGE_FP, 2, "back");
static NexButton continue_FP(NEXTION_PAGE_FP, 3, "next");
static NexText text_FP1(NEXTION_PAGE_FP, 4, "t0");
static NexText text_FP2(NEXTION_PAGE_FP, 5, "t1");
static NexText text_FP3(NEXTION_PAGE_FP, 6, "t2");

// PANTALLA POTENCIA TRIFASICA
#define NEXTION_PAGE_P 6
static NexPage page_P3(NEXTION_PAGE_P, NEXTION_PAGE_P, "page1");
static NexButton home_P3(NEXTION_PAGE_P, 2, "b0");
static NexText text_P31(NEXTION_PAGE_P, 3, "t0");
static NexText text_P32(NEXTION_PAGE_P, 4, "t1");
static NexText text_P33(NEXTION_PAGE_P, 5, "t2");

// PANTALLA POTENCIA REACTIVA TRIFASICA
#define NEXTION_PAGE_Q 7
static NexPage page_Q3(NEXTION_PAGE_Q, NEXTION_PAGE_Q, "page2");
static NexButton home_Q3(NEXTION_PAGE_Q, 2, "b0");
static NexText text_Q31(NEXTION_PAGE_Q, 3, "t0");
static NexText text_Q32(NEXTION_PAGE_Q, 4, "t1");
static NexText text_Q33(NEXTION_PAGE_Q, 5, "t2");

// PANTALLA POTENCIA APARENTE TRIFASICA
#define NEXTION_PAGE_S 8
static NexPage page_S3(NEXTION_PAGE_S, NEXTION_PAGE_S, "page3");
static NexButton home_S3(NEXTION_PAGE_S, 2, "b0");
static NexText text_S31(NEXTION_PAGE_S, 3, "t0");
static NexText text_S32(NEXTION_PAGE_S, 4, "t1");
static NexText text_S33(NEXTION_PAGE_S, 5, "t2");

#define NEXTION_FLAH_MODEL 5
int indices_totales;
int indice_actual;
static NexPage page_load(NEXTION_FLAH_MODEL, NEXTION_FLAH_MODEL, "page8");
static NexProgressBar load(NEXTION_FLAH_MODEL, 4, "j0");
static NexNumber porcentaje(NEXTION_FLAH_MODEL, 3, "n0");

/*
// PANTALLA VECTORES
NexPage page_Vector = NexPage(7, 7, "page7");
NexButton back_Vector = NexButton(7, 8, "b0");
NexButton continue_Vector = NexButton(7, 9, "b1");

NexGauge GaugeFase1 = NexGauge(7, 2, "z0");
NexGauge GaugeFase2 = NexGauge(7, 3, "z1");
NexGauge GaugeFase3 = NexGauge(7, 4, "z2");

NexText text_Vector1 = NexText(7, 5, "t0");
NexText text_Vector2 = NexText(7, 6, "t1");
NexText text_Vector3 = NexText(7, 7, "t2");
*/

// TOUCH
static NexTouch *nex_listen_list[] =
    {
        &back_TP,
        &continue_TP,

        &opcion_3P,
        &opcion_3Q,
        &opcion_3S,

        &home_P3,
        &home_Q3,
        &home_S3,

        &back_VN,
        &continue_VN,

        &back_I,
        &continue_I,

        &continue_FP,
        &back_FP,

        NULL};

void ChangePage(int page)
{
    switch (page)
    {
    case NEXTION_PAGE_PQS:
        page_TrianglePower.show();
        break;

    case NEXTION_PAGE_VLN:
        page_VoltajeNeutro.show();
        break;

    case NEXTION_PAGE_ILN:
        page_Corriente.show();
        break;

    case NEXTION_PAGE_FP:
        page_FP.show();
        break;
    }
}

void P3_PopCallback(void *ptr)
{
    page_P3.show();
    page = NEXTION_PAGE_P;
}

void Q3_PopCallback(void *ptr)
{
    page_Q3.show();
    page = NEXTION_PAGE_Q;
}

void S3_PopCallback(void *ptr)
{
    page_S3.show();
    page = NEXTION_PAGE_S;
}

void Home_PopCallback(void *ptr)
{
    page = 0;
    page_TrianglePower.show();
}

void DownPagePopCallback(void *ptr)
{
    page--;
    if (page < 0)
    {
        page = 3;
    }
    ChangePage(page);
}

void UpPagePopCallback(void *ptr)
{
    page++;
    if (page > 4)
    {
        page = 0;
    }
    ChangePage(page);
}

void UpdatePage(int page, PZEM_Trifasico Sensor)
{
    char buffer[20];
    uint32_t numberA;
    uint32_t numberB;
    uint32_t numberC;
    switch (page)
    {
    case NEXTION_PAGE_PQS:
        sprintf(buffer, "%d.%02d[kW]", (int)Sensor.P3, (int)(Sensor.P3 * 100) % 100);
        text_P3.setText(buffer);
        sprintf(buffer, "%d.%02d[kVAR]", (int)Sensor.Q3, (int)(Sensor.Q3 * 100) % 100);
        text_Q3.setText(buffer);
        sprintf(buffer, "%d.%02d[kVA]", (int)Sensor.S3, (int)(Sensor.S3 * 100) % 100);
        text_S3.setText(buffer);
        break;

    case NEXTION_PAGE_VLN:
        sprintf(buffer, "%d.%02d[V]", (int)Sensor.DatosFaseA.VLN, (int)(Sensor.DatosFaseA.VLN * 100) % 100);
        text_I1.setText(buffer);
        sprintf(buffer, "%d.%02d[V]", (int)Sensor.DatosFaseB.VLN, (int)(Sensor.DatosFaseB.VLN * 100) % 100);
        text_I2.setText(buffer);
        sprintf(buffer, "%d.%02d[V]", (int)Sensor.DatosFaseC.VLN, (int)(Sensor.DatosFaseC.VLN * 100) % 100);
        text_I3.setText(buffer);
        break;

    case NEXTION_PAGE_ILN:
        sprintf(buffer, "%d.%02d[A]", (int)Sensor.DatosFaseA.I, (int)(Sensor.DatosFaseA.I * 100) % 100);
        text_FP1.setText(buffer);
        sprintf(buffer, "%d.%02d[A]", (int)Sensor.DatosFaseB.I, (int)(Sensor.DatosFaseB.I * 100) % 100);
        text_FP2.setText(buffer);
        sprintf(buffer, "%d.%02d[A]", (int)Sensor.DatosFaseC.I, (int)(Sensor.DatosFaseC.I * 100) % 100);
        text_FP3.setText(buffer);
        break;

    case NEXTION_PAGE_FP:
        sprintf(buffer, "%d.%02d", (int)Sensor.DatosFaseA.FP, (int)(Sensor.DatosFaseA.FP * 100) % 100);
        text_I1.setText(buffer);
        sprintf(buffer, "%d.%02d", (int)Sensor.DatosFaseB.FP, (int)(Sensor.DatosFaseB.FP * 100) % 100);
        text_I2.setText(buffer);
        sprintf(buffer, "%d.%02d", (int)Sensor.DatosFaseC.FP, (int)(Sensor.DatosFaseC.FP * 100) % 100);
        text_I3.setText(buffer);
        break;

    case 10:
        // uint32_t aA = (int)Sensor.DatosFaseA.angule * (180 / PI);
        // uint32_t aB = (int)Sensor.DatosFaseB.angule * (180 / PI);
        // uint32_t aC = (int)Sensor.DatosFaseC.angule * (180 / PI);

        numberA = Sensor.DatosFaseA.angule * (180 / PI);
        numberB = Sensor.DatosFaseB.angule * (180 / PI);
        numberC = Sensor.DatosFaseC.angule * (180 / PI);

        Serial.println(numberA);
        Serial.println(numberB);
        Serial.println(numberC);

        /*
        GaugeFase1.setValue(numberA);
        GaugeFase2.setValue(numberB);
        GaugeFase3.setValue(numberC);

        sprintf(buffer, "%d°", (int)numberA);
        text_Vector1.setText(buffer);

        sprintf(buffer, "%d°", (int)numberA);
        text_Vector2.setText(buffer);

        sprintf(buffer, "%d°", (int)numberA);
        text_Vector3.setText(buffer);

        */
        break;

    case NEXTION_PAGE_P:
        sprintf(buffer, "%d.%02d[kW]", (int)Sensor.DatosFaseA.P, (int)(Sensor.DatosFaseA.P * 100) % 100);
        text_I1.setText(buffer);
        sprintf(buffer, "%d.%02d[kW]", (int)Sensor.DatosFaseB.P, (int)(Sensor.DatosFaseB.P * 100) % 100);
        text_I2.setText(buffer);
        sprintf(buffer, "%d.%02d[kW]", (int)Sensor.DatosFaseC.P, (int)(Sensor.DatosFaseC.P * 100) % 100);
        text_I3.setText(buffer);
        break;

    case NEXTION_PAGE_Q:
        sprintf(buffer, "%d.%02d[kVAR]", (int)Sensor.DatosFaseA.Q, (int)(Sensor.DatosFaseA.Q * 100) % 100);
        text_I1.setText(buffer);
        sprintf(buffer, "%d.%02d[kVAR]", (int)Sensor.DatosFaseB.Q, (int)(Sensor.DatosFaseB.Q * 100) % 100);
        text_I2.setText(buffer);
        sprintf(buffer, "%d.%02d[kVAR]", (int)Sensor.DatosFaseC.Q, (int)(Sensor.DatosFaseC.Q * 100) % 100);
        text_I3.setText(buffer);
        break;

    case NEXTION_PAGE_S:
        sprintf(buffer, "%d.%02d[kVA]", (int)Sensor.DatosFaseA.S, (int)(Sensor.DatosFaseA.S * 100) % 100);
        text_I1.setText(buffer);
        sprintf(buffer, "%d.%02d[kVA]", (int)Sensor.DatosFaseB.S, (int)(Sensor.DatosFaseB.S * 100) % 100);
        text_I2.setText(buffer);
        sprintf(buffer, "%d.%02d[kVA]", (int)Sensor.DatosFaseC.S, (int)(Sensor.DatosFaseC.S * 100) % 100);
        text_I3.setText(buffer);
        break;
    }
}

void NextionUI_initialize()
{
    nexSerial.begin(115200, SERIAL_8N1, NEXTION_SERIAL_RX, NEXTION_SERIAL_TX);
    delay(1000);

    // CONFIGURAMOS LOS BOTONES PARA DESPLAZARNOS
    back_TP.attachPop(DownPagePopCallback, &back_TP);
    continue_TP.attachPop(UpPagePopCallback, &continue_TP);

    back_VN.attachPop(DownPagePopCallback, &back_VN);
    continue_VN.attachPop(UpPagePopCallback, &continue_VN);

    back_I.attachPop(DownPagePopCallback, &back_I);
    continue_I.attachPop(UpPagePopCallback, &continue_I);

    back_FP.attachPop(DownPagePopCallback, &back_FP);
    continue_FP.attachPop(UpPagePopCallback, &continue_FP);

    // back_Vector.attachPop(DownPagePopCallback, &back_I);
    // continue_Vector.attachPop(UpPagePopCallback, &continue_I);

    // CONFIGURAMOS LOS BOTONES DE LAS POTENCIAS
    opcion_3P.attachPop(P3_PopCallback, &opcion_3P);
    opcion_3Q.attachPop(Q3_PopCallback, &opcion_3Q);
    opcion_3S.attachPop(S3_PopCallback, &opcion_3S);

    home_P3.attachPop(Home_PopCallback, &home_P3);
    home_Q3.attachPop(Home_PopCallback, &home_Q3);
    home_S3.attachPop(Home_PopCallback, &home_S3);

    page_TrianglePower.show();
}

void NextionUI_runEvents(PZEM_Trifasico Sensor)
// Esta funcion se encarga de habilitar los eventos de la pantalla
{
    if (_globalPause)
        return;
    if (!_FlashModel)
    {
        UpdatePage(page, Sensor);
    }
    else
    {
        NextionUI_flah_model(indice_actual);
    }
    nexLoop(nex_listen_list);
}

static void NextionUI_globalPause(bool p)
{
    _globalPause = p;
}

void NextionUI_flah_model(int indice, bool initialize)
{
    if (initialize)
    {
        indices_totales = indice;
        Serial.println("Flash Comienza");
        _FlashModel = true;
        page_load.show();
    }
    else
    {
        int avance = (indice * 100) / indices_totales;
        load.setValue(avance);
        porcentaje.setValue(avance);
    }
}

void NextionUI_NextIndice(int indice)
{
    indice_actual = indice;
}