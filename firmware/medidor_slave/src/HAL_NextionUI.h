#pragma once

#include <Arduino.h>
#include "PZEM_Trifasico.h"
#include <ESPAsyncWebServer.h>

void NextionUI_NextIndice(int indice);
void NextionUI_flah_model(int indice,bool initialize = false, bool finish_god = false, bool finish_bad = false);

void NextionUI_initialize(AsyncWebServer & srv);
void NextionUI_runEvents(PZEM_Trifasico Sensor);




