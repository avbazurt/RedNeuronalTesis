#pragma once

#include "SPI.h"
#include "SD.h"
#include <esp_now.h>
#include <RTClib.h>
#include "Preferences.h"

#include "HAL_NextionUI.h"
#include "Nextion.h"

#include <YuboxOTAClass.h>
#include "YuboxOTA_Flasher_Nextion.h"

#include "HAL_config.h"

// Pausa global de interacción, para flasheo de Nextion
static bool _globalPause = false;
static bool _FlashModel = false;
//

#define MAX_PANTALLA 6

#define PANTALLA_VLN 0
#define PANTALLA_IL 1
#define PANTALLA_FP 2
#define PANTALLA_3P 3
#define PANTALLA_3Q 4
#define PANTALLA_3S 5
#define PANTALLA_3PQS 6

#define PANTALLA_MENU 7

int HMI_page = 0;
bool _MaxValue = false;

// PANTALLA INFORMACION
#define NEXTION_PAGE_MAIN 0
static NexPage page_main(NEXTION_PAGE_MAIN, NEXTION_PAGE_MAIN, "page0");
static NexText text_menu(NEXTION_PAGE_MAIN, 6, "t0");
static NexText text_hora(NEXTION_PAGE_MAIN, 7, "t1");

static NexButton back(NEXTION_PAGE_MAIN, 3, "back");
static NexButton next(NEXTION_PAGE_MAIN, 4, "next");
static NexButton maximo(NEXTION_PAGE_MAIN, 2, "max");
static NexButton menu(NEXTION_PAGE_MAIN, 5, "menu");

static NexText text_1(NEXTION_PAGE_MAIN, 8, "t2");
static NexText text_2(NEXTION_PAGE_MAIN, 9, "t3");
static NexText text_3(NEXTION_PAGE_MAIN, 10, "t4");

static NexText dato_1(NEXTION_PAGE_MAIN, 11, "t5");
static NexText dato_2(NEXTION_PAGE_MAIN, 12, "t6");
static NexText dato_3(NEXTION_PAGE_MAIN, 13, "t7");

#define NEXTION_FLAH_MODEL 2
int indices_totales;
int indice_actual;
static NexPage page_load(NEXTION_FLAH_MODEL, NEXTION_FLAH_MODEL, "page1");
static NexProgressBar load(NEXTION_FLAH_MODEL, 4, "j0");
static NexNumber porcentaje(NEXTION_FLAH_MODEL, 3, "n0");

// TOUCH
static NexTouch *nex_listen_list[] =
    {
        &back,
        &next,
        &maximo,
        &menu,
        NULL};

static void NextionUI_globalPause(bool p)
{
    _globalPause = p;
}

static YuboxOTA_Flasher *_createNextionFlasher(void)
{
    esp_now_deinit();
    YuboxOTA_Flasher_Nextion *f = new YuboxOTA_Flasher_Nextion();
    f->attachNextion(std::bind(&NextionUI_globalPause, std::placeholders::_1), &nexSerial);
    return f;
}

static void NextionUI_UpdateParametros(int page)
{
    // Esta funcion se encarga de actualizar los parametros de la pantalla que solo se requiere una vez

    if (page >= PANTALLA_VLN and page <= PANTALLA_3S)
    {
        text_1.setText("L1");
        text_2.setText("L2");
        text_3.setText("L3");
        switch (page)
        {
        case PANTALLA_VLN:
            text_menu.setText("VLN-INSTANT");
            break;

        case PANTALLA_IL:
            text_menu.setText("IL-INSTANT");
            break;

        case PANTALLA_FP:
            text_menu.setText("FP-INSTANT");
            break;

        case PANTALLA_3P:
            text_menu.setText("3P-INSTANT");
            break;

        case PANTALLA_3Q:
            text_menu.setText("3Q-INSTANT");
            break;

        case PANTALLA_3S:
            text_menu.setText("3S-INSTANT");
            break;
        }
    }
    else
    {
        Preferences flashMemory;
        String fecha_hora;
        switch (page)
        {
        case PANTALLA_3PQS:
            text_menu.setText("3PQS-INSTANT");
            text_1.setText("P3");
            text_2.setText("Q3");
            text_3.setText("S3");
            break;

        case PANTALLA_MENU:
            text_menu.setText("MENU");

            maximo.Set_background_crop_picc(1);
            maximo.Set_background_image_pic(1);
            maximo.setText("");

            next.Set_background_crop_picc(1);
            next.Set_background_image_pic(1);

            back.Set_background_crop_picc(1);
            back.Set_background_image_pic(1);

            menu.setText("BACK");

            text_1.setText("");
            text_1.setText("");
            text_1.setText("");

            text_1.setFont(5);
            text_2.setFont(5);
            text_3.setFont(5);

            text_1.setText("MAC");
            text_2.setText("IP");
            text_3.setText("LAST");

            dato_1.setText("");
            dato_2.setText("");
            dato_3.setText("");

            dato_1.setFont(5);
            dato_2.setFont(5);
            dato_3.setFont(5);

            dato_1.setText(WiFi.macAddress().c_str());
            dato_2.setText("192.168.4.1");

            flashMemory.begin("model", false);
            fecha_hora = flashMemory.getString("date_flash", "");
            dato_3.setText(fecha_hora.c_str());
            flashMemory.end();
            break;

        default:
            break;
        }
    }
}
void buttonNavigate(void *ptr)
{
    NexButton *btn = (NexButton *)ptr;
    uint32_t font;
    btn->getFont(&font);
    if (font == 5)
    {
        String text;
        btn->getText(text);

        if (text == "MAX")
        {
            _MaxValue = !_MaxValue;
            if (_MaxValue)
            {
                maximo.Set_background_color_bco(255);
            }
            else
            {
                maximo.Set_background_color_bco(266);
            }
        }
        else if (text == "MENU")
        {
            HMI_page = PANTALLA_MENU;
            NextionUI_UpdateParametros(HMI_page);
        }
        else if (text == "BACK")
        {
            maximo.Set_background_crop_picc(0);
            maximo.Set_background_image_pic(0);
            maximo.setText("MAX");

            next.Set_background_crop_picc(0);
            next.Set_background_image_pic(0);

            back.Set_background_crop_picc(0);
            back.Set_background_image_pic(0);

            text_1.setFont(7);
            text_2.setFont(7);
            text_3.setFont(7);

            dato_1.setFont(7);
            dato_2.setFont(7);
            dato_3.setFont(7);

            menu.setText("MENU");

            HMI_page = PANTALLA_VLN;
            NextionUI_UpdateParametros(HMI_page);
        }
    }
    else if (font == 0 or font == 1 and HMI_page != PANTALLA_MENU)
    {
        if (font == 0)
        { // Esta condicion nos indica que se debe pasar el siguiente pantalla
            HMI_page++;
            if (HMI_page > MAX_PANTALLA)
            {
                HMI_page = 0;
            }
        }

        else if (font == 1)
        { // Esta condicion nos indica que se debe pasar al anterior pantalla
            HMI_page--;
            if (HMI_page < 0)
            {
                HMI_page = MAX_PANTALLA;
            }
        }
        NextionUI_UpdateParametros(HMI_page);
    }
}

void FormatData(NexText text, float valor, float valorMax, String unidad)
{
    char buffer[100];
    float dato;

    if (_MaxValue)
    {
        dato = valorMax;
    }
    else
    {
        dato = valor;
    }

    sprintf(buffer, "%d.%02d%s", (int)dato, (int)(dato * 100) % 100, unidad);
    text.setText(buffer);
}

static void NextionUI_UpdateData(int page, PZEM_Trifasico Sensor)
{
    struct tm timeinfo;
    time_t ts_ahora;

    // ¿Qué hora es? Se asume hora sistema correcta vía NTP
    ts_ahora = time(NULL);
    localtime_r(&ts_ahora, &timeinfo);

    char buffer[100];
    sprintf(buffer, "%0.2d:%0.2d:%0.2d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    text_hora.setText(buffer);

//*************************** EXPERIMENTO **************************
#ifndef SIMULTATION_MATLAB
    Sensor.GetMedicionTrifasica();
#endif
//*************************** EXPERIMENTO **************************
    
    switch (page)
    {
    case PANTALLA_VLN:
        FormatData(dato_1, Sensor.DatosFaseA.VLN, Sensor.MaxFaseA.VLN, "[V]");
        FormatData(dato_2, Sensor.DatosFaseB.VLN, Sensor.MaxFaseB.VLN, "[V]");
        FormatData(dato_3, Sensor.DatosFaseC.VLN, Sensor.MaxFaseB.VLN, "[V]");
        break;

    case PANTALLA_IL:
        FormatData(dato_1, Sensor.DatosFaseA.I, Sensor.MaxFaseA.I, "[A]");
        FormatData(dato_2, Sensor.DatosFaseB.I, Sensor.MaxFaseB.I, "[A]");
        FormatData(dato_3, Sensor.DatosFaseC.I, Sensor.MaxFaseC.I, "[A]");
        break;

    case PANTALLA_FP:
        FormatData(dato_1, Sensor.DatosFaseA.FP, Sensor.MaxFaseA.FP, "");
        FormatData(dato_2, Sensor.DatosFaseB.FP, Sensor.MaxFaseB.FP, "");
        FormatData(dato_3, Sensor.DatosFaseC.FP, Sensor.MaxFaseC.FP, "");
        break;

    case PANTALLA_3P:
        FormatData(dato_1, Sensor.DatosFaseA.P, Sensor.MaxFaseA.P, "[W]");
        FormatData(dato_2, Sensor.DatosFaseB.P, Sensor.MaxFaseB.P, "[W]");
        FormatData(dato_3, Sensor.DatosFaseC.P, Sensor.MaxFaseC.P, "[W]");
        break;

    case PANTALLA_3Q:
        FormatData(dato_1, Sensor.DatosFaseA.Q, Sensor.MaxFaseA.Q, "[VAR]");
        FormatData(dato_2, Sensor.DatosFaseB.Q, Sensor.MaxFaseB.Q, "[VAR]");
        FormatData(dato_3, Sensor.DatosFaseC.Q, Sensor.MaxFaseC.Q, "[VAR]");
        break;

    case PANTALLA_3S:
        FormatData(dato_1, Sensor.DatosFaseA.S, Sensor.MaxFaseA.S, "[VA]");
        FormatData(dato_2, Sensor.DatosFaseB.S, Sensor.MaxFaseB.S, "[VA]");
        FormatData(dato_3, Sensor.DatosFaseC.S, Sensor.MaxFaseC.S, "[VA]");
        break;

    case PANTALLA_3PQS:
        FormatData(dato_1, Sensor.TriFase.P3, Sensor.TriFase.P3_MAX, "[kW]");
        FormatData(dato_2, Sensor.TriFase.Q3, Sensor.TriFase.Q3_MAX, "[kVAR]");
        FormatData(dato_3, Sensor.TriFase.S3, Sensor.TriFase.S3_MAX, "[kVA]");
        break;

    default:
        break;
    }

}

void NextionUI_initialize(AsyncWebServer &srv)
{
    nexSerial.begin(115200, SERIAL_8N1, NEXTION_SERIAL_RX, NEXTION_SERIAL_TX);
    delay(1000);

    back.attachPop(buttonNavigate, &back);
    next.attachPop(buttonNavigate, &next);
    maximo.attachPop(buttonNavigate, &maximo);
    menu.attachPop(buttonNavigate, &menu);

    page_main.show();
    NextionUI_UpdateParametros(HMI_page);

    YuboxOTA.addFirmwareFlasher(srv, "nextion", "Nextion TFT Firmware",
                                std::bind(&_createNextionFlasher));
}

void NextionUI_runEvents(PZEM_Trifasico Sensor)
// Esta funcion se encarga de habilitar los eventos de la pantalla
{
    if (_globalPause)
        return;
    if (!_FlashModel)
    {
        NextionUI_UpdateData(HMI_page, Sensor);
    }
    else
    {
        NextionUI_flah_model(indice_actual);
    }
    nexLoop(nex_listen_list);
}

void NextionUI_flah_model(int indice, bool initialize, bool finish_god, bool finish_bad)
{
    if (finish_god)
    {
        // load.Set_background_color_bco(250);
        porcentaje.setValue(100);
        load.setValue(100);

        load.Set_font_color_pco(5600);
        nexLoop(nex_listen_list);
        return;
    }

    if (finish_bad)
    {
        load.Set_font_color_pco(9000);
        porcentaje.setValue(0);

        vTaskDelay(2000 / portTICK_RATE_MS);

        _FlashModel = false;
        page_main.show();
        NextionUI_UpdateParametros(HMI_page);
        return;
    }

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
