#pragma once
#include "SPI.h"
#include "SD.h"
#include "HAL_sensor.h"
#include "Nextion.h"

void NextionUI_NextIndice(int indice);
void NextionUI_flah_model(int indice,bool initialize = false);

void NextionUI_initialize();
void NextionUI_runEvents(MedidorTrifasico Sensor);




