#pragma once
#include <YuboxSimple.h>
#include <Preferences.h>

// Definicion YUBOX FRAMEWORK
#define ARDUINOJSON_USE_LONG_LONG 1

AsyncEventSource eventosLector("/yubox-api/lectura/events");
 

void routeHandler_yuboxAPI_calibration_tempoffset_GET(AsyncWebServerRequest *);
void routeHandler_yuboxAPI_calibration_tempoffset_POST(AsyncWebServerRequest *);

void routeHandler_yuboxAPI_contactores_releSet_GET(AsyncWebServerRequest *);
void routeHandler_yuboxAPI_contactores_releSet_POST(AsyncWebServerRequest *);

void routeHandler_yuboxAPI_espnow_config_GET(AsyncWebServerRequest *);
void routeHandler_yuboxAPI_espnow_config_POST(AsyncWebServerRequest *);

