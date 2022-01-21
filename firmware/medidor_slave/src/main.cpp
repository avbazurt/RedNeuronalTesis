#include <Arduino.h>
#include "NeuralNetwork.h"
#include "PZEM_Trifasico.h"

#include <TaskScheduler.h>
#include <RTClib.h>
#include "PCA9536.h"

#include "HAL_Yubox_Framework.h"
#include "HAL_now.h"
#include "HAL_NextionUI.h"

#include "HAL_config.h"

Scheduler mainScheduler;

// Tareas
void yuboxMuestrearFases(void);
Task task_yuboxMuestrearFases(TASK_SECOND * 1, TASK_FOREVER, &yuboxMuestrearFases, &mainScheduler);

// Tareas
void espnowMuestrearFases(void);
Task task_espnowMuestrearFases(TASK_SECOND * 2, TASK_FOREVER, &espnowMuestrearFases, &mainScheduler);

void TaskPredition();
Task task_predition(2 * TASK_SECOND, TASK_FOREVER, &TaskPredition, &mainScheduler);

// Actualizar otra vez RTC cada 15 minutos
void yuboxUpdateRTC(void);
Task task_yuboxUpdateRTC(TASK_SECOND * 10, TASK_FOREVER, &yuboxUpdateRTC, &mainScheduler);

void mostrarHora(void);

// Clases Necesarias
ESP32_now *now;
NeuralNetwork *nn;
PZEM_Trifasico *sensor;

// Objeto Rele i2C
PCA9536 *releI2C;
bool enable_modo_auto;

bool on_rele_A = false;
bool on_rele_B = false;
bool on_rele_C = false;

float set_point_fp;

bool status_muestreo;
int second_muestreo;

// Save Flash Memory
#define addres_date_flash "date_flash"
#define addres_set_point "sp_fp"
#define addres_enable_sp "enable_sp"

#define namespace_configuration "configuration"
#define time_muestra "muestra"
#define addres_mac_1 "mac_1"
#define addres_mac_2 "mac_2"
#define addres_mac_3 "mac_3"
#define addres_mac_4 "mac_4"
#define addres_mac_5 "mac_5"
#define addres_mac_6 "mac_6"

// Variables Configurables Yubox
Preferences flashMemory;

// Objeto que controla el RTC DS3231
RTC_DS3231 rtc;
bool rtc_valid = false;
void iniciarHoraDesdeRTC(void);

// ESPNOW
uint8_t MAC_servidor[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Callback
void ReciveDataNow(char MAC[], char text[]);
int histeresis(float valor, float tp);

void setup()
{
  NextionUI_initialize(yubox_HTTPServer);
  Serial.begin(115200);
  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);

  flashMemory.begin("model", false);
  set_point_fp = flashMemory.getFloat(addres_set_point, 0.95);
  enable_modo_auto = flashMemory.getBool(addres_enable_sp, false);
  flashMemory.end();

  delay(500);

  flashMemory.begin(namespace_configuration, false);
  second_muestreo = flashMemory.getInt(time_muestra, 0);
  MAC_servidor[0] = flashMemory.getInt(addres_mac_1, 0);
  MAC_servidor[1] = flashMemory.getInt(addres_mac_2, 0);
  MAC_servidor[2] = flashMemory.getInt(addres_mac_3, 0);
  MAC_servidor[3] = flashMemory.getInt(addres_mac_4, 0);
  MAC_servidor[4] = flashMemory.getInt(addres_mac_5, 0);
  MAC_servidor[5] = flashMemory.getInt(addres_mac_6, 0);
  flashMemory.end();

  Serial.println(enable_modo_auto);

  iniciarHoraDesdeRTC();
  releI2C = new PCA9536();
  releI2C->setup();

  sensor = new PZEM_Trifasico(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN,
                              PZEM_ADDRESS_FASE_A,
                              PZEM_ADDRESS_FASE_B,
                              PZEM_ADDRESS_FASE_C);

  sensor->setModeSimulation();

  nn = new NeuralNetwork();

  yuboxAddManagedHandler(&eventosLector);
  yubox_HTTPServer.on("/yubox-api/contactores/tempoffset.json", HTTP_GET, routeHandler_yuboxAPI_calibration_tempoffset_GET);
  yubox_HTTPServer.on("/yubox-api/contactores/tempoffset.json", HTTP_POST, routeHandler_yuboxAPI_calibration_tempoffset_POST);

  yubox_HTTPServer.on("/yubox-api/contactores/releSet.json", HTTP_POST, routeHandler_yuboxAPI_contactores_releSet_POST);
  yubox_HTTPServer.on("/yubox-api/contactores/releSet.json", HTTP_GET, routeHandler_yuboxAPI_contactores_releSet_GET);

  yubox_HTTPServer.on("/yubox-api/espnow/config.json", HTTP_POST, routeHandler_yuboxAPI_espnow_config_POST);
  yubox_HTTPServer.on("/yubox-api/espnow/config.json", HTTP_GET, routeHandler_yuboxAPI_espnow_config_GET);

  yuboxSimpleSetup();

  YuboxWiFi.releaseControlOfWiFi();
  YuboxWiFi.saveControlOfWiFi();

  now = new ESP32_now();
  now->setReciveCallback(ReciveDataNow);
  now->begin();

  // Actualización de RTC
  task_yuboxUpdateRTC.enable();
  task_predition.enable();
  task_yuboxMuestrearFases.enable();

  if (second_muestreo > 0)
  {
    task_espnowMuestrearFases.setInterval(second_muestreo * TASK_SECOND);
    task_espnowMuestrearFases.enable();
  }
}

void loop()
{
  yuboxSimpleLoopTask();
  NextionUI_runEvents(*sensor);
  mainScheduler.execute();
}

void espnowMuestrearFases(void)
{
  // uint8_t peerAddress[] = {0x98, 0xF4, 0xAB, 0x1B, 0x1E, 0xA0};
  status_muestreo = now->sentData(MAC_servidor, "hola");
}

void yuboxMuestrearFases(void)
{

  DynamicJsonDocument json_tablerow(JSON_OBJECT_SIZE(13) + 2 * JSON_OBJECT_SIZE(1));
  // NOTA: se usa bucle para dejar listo para soporte de múltiple fases
  String json_tableOutput = "[";
  for (auto i = 0; i < 1; i++)
  {
    if (json_tableOutput.length() > 1)
      json_tableOutput += ",";

    // Potencias y factor de potencia
    json_tablerow["apparent_power"] = random(1, 100);
    json_tablerow["real_power"] = random(1, 100);
    json_tablerow["reactive_power"] = random(1, 100);

    DynamicJsonDocument d(JSON_OBJECT_SIZE(1));

    json_tablerow["voltage_a"] = sensor->DatosFaseA.VLN;
    json_tablerow["voltage_b"] = sensor->DatosFaseB.VLN;
    json_tablerow["voltage_c"] = sensor->DatosFaseC.VLN;

    json_tablerow["current_a"] = sensor->DatosFaseA.I;
    json_tablerow["current_b"] = sensor->DatosFaseB.I;
    json_tablerow["current_c"] = sensor->DatosFaseC.I;

    json_tablerow["power_factor_a"] = sensor->DatosFaseA.FP;
    json_tablerow["power_factor_b"] = sensor->DatosFaseB.FP;
    json_tablerow["power_factor_c"] = sensor->DatosFaseC.FP;

    serializeJson(json_tablerow, json_tableOutput);
  }
  json_tableOutput += "]";

  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(3));
  json_doc["event"] = "EnergyReport";
  json_doc["ts"] = 1000ULL * YuboxNTPConf.getUTCTime();
  json_doc["phases"] = serialized(json_tableOutput.c_str());

  String json_output;
  serializeJson(json_doc, json_output);

  if (eventosLector.count() > 0)
  {
    eventosLector.send(json_output.c_str(), "EnergyReport", 0, 1000);
  }
}

void TaskPredition()
{

  if (enable_modo_auto)
  {
    struct tm timeinfo;
    time_t ts_ahora;

    // ¿Qué hora es? Se asume hora sistema correcta vía NTP
    ts_ahora = time(NULL);
    localtime_r(&ts_ahora, &timeinfo);

    // Calcular milisegundos desde la medianoche para la hora actual y para cada
    // uno de los instantes de alimentación.
#define SEC_MEDIANOCHE(HH, MM, SS) ((SS + 60 * (MM + 60 * HH)))
#define SEC_EN_DIA 86400000

    uint32_t sec_ahora = SEC_MEDIANOCHE(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    float seconds = sec_ahora;

    float day = timeinfo.tm_mday + 1;
    float wekday = timeinfo.tm_wday + 1;

    sensor->GetMedicionTrifasica();

    nn->getInputBuffer()[0] = wekday;
    nn->getInputBuffer()[1] = day;
    nn->getInputBuffer()[2] = sec_ahora;

    nn->getInputBuffer()[3] = sensor->DatosFaseA.VLN;
    nn->getInputBuffer()[4] = sensor->DatosFaseB.VLN;
    nn->getInputBuffer()[5] = sensor->DatosFaseC.VLN;

    nn->getInputBuffer()[6] = wekday;
    nn->getInputBuffer()[7] = day;
    nn->getInputBuffer()[8] = sec_ahora;

    nn->getInputBuffer()[9] = sensor->DatosFaseA.I;
    nn->getInputBuffer()[10] = sensor->DatosFaseB.I;
    nn->getInputBuffer()[11] = sensor->DatosFaseC.I;

    float *result = nn->getOutputBuffer();
    on_rele_A = result[0] >= set_point_fp;
    on_rele_B = result[1] >= set_point_fp;
    on_rele_C = result[2] >= set_point_fp;
  }

  int rele_faseA = 0b00000001 * ((int)(on_rele_A));
  int rele_faseB = 0b00000010 * ((int)(on_rele_B));
  int rele_faseC = 0b00000100 * ((int)(on_rele_C));
  int output_rele = rele_faseA | rele_faseB | rele_faseC;

  releI2C->output(output_rele);
}

void ReciveDataNow(char MAC[], char text[])
{
  Serial.printf("Received message from: %s - %s\n", MAC, text);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, text);

  // Test if parsing succeeds.
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  const char *opcion = doc["cmd"];
  if (String(opcion) == "new_model")
  {
    int total_indice = doc["len"];
    nn->len_new_model_tflite = total_indice;
    Serial.print("Creando el Array");
    Serial.println(total_indice);

    nn->new_model_tflite = new char[total_indice];
    task_predition.disable();
    task_yuboxMuestrearFases.disable();
    NextionUI_flah_model(total_indice, true);
    nn->indice_new_model = 0;
  }

  else if (String(opcion) == "model")
  {
    const char *valor = doc["valor"];
    int indice = doc["indice"];

    int valor_entero = (long)strtol(valor, 0, 16);
    nn->new_model_tflite[indice] = valor_entero;
    Serial.println(nn->new_model_tflite[indice], HEX);
    NextionUI_NextIndice(indice);
    nn->indice_new_model++;
  }

  else
  {
    Serial.println("--------------------");
    Serial.println("Resumen de Flasheo: ");
    Serial.printf("Datos guardados: %d\n", nn->indice_new_model);
    Serial.printf("Tamaño total del buffer: %d\n", nn->len_new_model_tflite);

    if (nn->indice_new_model == nn->len_new_model_tflite)
    {
      Serial.println("Flasheando modelo");

      struct tm timeinfo;
      time_t ts_ahora;

      flashMemory.begin("model", false);

      // ¿Qué hora es? Se asume hora sistema correcta vía NTP
      ts_ahora = time(NULL);
      localtime_r(&ts_ahora, &timeinfo);

      char buffer[100];
      sprintf(buffer, "%0.2d/%0.2d/%0.4d %0.2d:%0.2d:%0.2d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      flashMemory.putString("date_flash", String(buffer));
      flashMemory.end();

      nn->SaveModel();
    }
    else
    {
      Serial.println("Error Flashear modelo");
    }
    ESP.restart();
  }
}

void iniciarHoraDesdeRTC(void)
{
  // Inicialización y verificación de presencia de RTC
  rtc_valid = rtc.begin();
  if (!rtc_valid)
  {
    Serial.println("no se dispone de RTC! Se continúa sin fecha hasta obtener NTP.");
  }
  else
  {
    Serial.println("RTC DS3231 detectado correctamente...");
    // Para primera inicialización, se ajusta fecha y hora
    if (rtc.lostPower())
    {
      Serial.println("RTC DS3231 perdió energía o primer arranque, se ajusta...");
      // TODO: warm reset puede haber preservado hora interna, debería aprovecharse

      // Fijar a fecha y hora de compilacion - SE ASUME HORA UTC POR AHORA
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    else
    {
      Serial.println("Iniciar hasta recibir NTP.");

      // Indicar como valor de RTC lo que reporte el chip, hasta recibir NTP
      DateTime rtc_now = rtc.now();
      YuboxNTPConf.setRTCHint(rtc_now.unixtime());
    }
  }
}

void yuboxUpdateRTC(void)
{
  if (!rtc_valid)
    return; // Sólo actualizar si se tiene RTC
  Serial.println("Actualizar Hora");
  rtc.adjust(DateTime(time(NULL)));
}

void routeHandler_yuboxAPI_calibration_tempoffset_GET(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(3));

  json_doc["enabled"] = enable_modo_auto;
  json_doc["sp_fp"] = set_point_fp;

  response->setCode(200);
  serializeJson(json_doc, *response);
  request->send(response);
}

void routeHandler_yuboxAPI_calibration_tempoffset_POST(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  bool clientError = false;
  bool serverError = false;
  String responseMsg = "";
  AsyncWebParameter *p;

  bool n_enabled = enable_modo_auto;
  double n_sp_fp = set_point_fp;

  if (!clientError && request->hasParam("enabled", true))
  {
    p = request->getParam("enabled", true);
    if (p->value() == "1")
    {
      n_enabled = true;
    }
    else if (p->value() == "0")
    {
      n_enabled = false;
    }
    else
    {
      clientError = true;
      responseMsg = "Bandera de activar corrección temperatura, debe ser 1 o 0";
    }
  }

#define ASSIGN_FROM_POST(TAG, FMT)                       \
  if (!clientError && request->hasParam(#TAG, true))     \
  {                                                      \
    p = request->getParam(#TAG, true);                   \
    int n = sscanf(p->value().c_str(), FMT, &(n_##TAG)); \
    if (n <= 0)                                          \
    {                                                    \
      clientError = true;                                \
      responseMsg = "Formato numérico incorrecto para "; \
      responseMsg += #TAG;                               \
    }                                                    \
  }

  ASSIGN_FROM_POST(sp_fp, "%lf")

  Serial.println(n_enabled);
  Serial.println(n_sp_fp);

  if (n_sp_fp > 1)
  {
    serverError = true;
    responseMsg = "El factor de potencia no puede ser mayor a 1";
  }
  else if (n_sp_fp < 0)
  {
    serverError = true;
    responseMsg = "El factor de potencia no puede ser un valor negativo";
  }

  if (!clientError)
  {
    enable_modo_auto = n_enabled;
    set_point_fp = n_sp_fp;

    flashMemory.begin("model", false);
    flashMemory.putBool(addres_set_point, n_enabled);
    flashMemory.putFloat(addres_set_point, n_sp_fp);
    flashMemory.end();
  }

  if (!clientError && !serverError)
  {
    responseMsg = "Parámetros actualizados correctamente";
  }
  unsigned int httpCode = 200;
  if (clientError)
    httpCode = 400;
  if (serverError)
    httpCode = 500;

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(httpCode);
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(2));
  json_doc["success"] = !(clientError || serverError);
  json_doc["msg"] = responseMsg.c_str();

  serializeJson(json_doc, *response);
  request->send(response);
}

void routeHandler_yuboxAPI_contactores_releSet_GET(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(3));

  json_doc["enabled_faseA"] = on_rele_A;
  json_doc["enabled_faseB"] = on_rele_B;
  json_doc["enabled_faseC"] = on_rele_C;

  response->setCode(200);
  serializeJson(json_doc, *response);
  request->send(response);
}

void routeHandler_yuboxAPI_contactores_releSet_POST(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  bool clientError = false;
  bool serverError = false;
  String responseMsg = "";
  AsyncWebParameter *p;

  bool n_releA = false;
  bool n_releB = false;
  bool n_releC = false;

  if (!clientError && request->hasParam("releA", true))
  {
    p = request->getParam("releA", true);
    if (p->value() == "1")
    {
      n_releA = true;
    }
    else if (p->value() == "0")
    {
      n_releA = false;
    }
    else
    {
      clientError = true;
      responseMsg = "Bandera de activar corrección temperatura, debe ser 1 o 0";
    }
  }

  if (!clientError && request->hasParam("releB", true))
  {
    p = request->getParam("releB", true);
    if (p->value() == "1")
    {
      n_releB = true;
    }
    else if (p->value() == "0")
    {
      n_releB = false;
    }
    else
    {
      clientError = true;
      responseMsg = "Bandera de activar corrección temperatura, debe ser 1 o 0";
    }
  }

  if (!clientError && request->hasParam("releC", true))
  {
    p = request->getParam("releC", true);
    if (p->value() == "1")
    {
      n_releC = true;
    }
    else if (p->value() == "0")
    {
      n_releC = false;
    }
    else
    {
      clientError = true;
      responseMsg = "Bandera de activar corrección temperatura, debe ser 1 o 0";
    }
  }

  if (enable_modo_auto)
  {
    // Si esta en modo Auto no se pueden controlar manualemnte los rele
    serverError = true;
    responseMsg = "No se pueden controlar manualemnte los reles en modo automatico";
  }
  else
  {
    on_rele_A = n_releA;
    on_rele_B = n_releB;
    on_rele_C = n_releC;

    Serial.println(n_releA);
    Serial.println(n_releB);
    Serial.println(n_releC);
  }

  if (!clientError && !serverError)
  {
    responseMsg = "Parámetros actualizados correctamente";
  }
  unsigned int httpCode = 200;
  if (clientError)
    httpCode = 400;
  if (serverError)
    httpCode = 500;

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(httpCode);
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(2));
  json_doc["success"] = !(clientError || serverError);
  json_doc["msg"] = responseMsg.c_str();

  serializeJson(json_doc, *response);
  request->send(response);
}

void routeHandler_yuboxAPI_espnow_config_GET(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(7));

  char mac[50];
  sprintf(mac, "%0.2X:%0.2X:%0.2X:%0.2X:%0.2X:%0.2X", MAC_servidor[0], MAC_servidor[1], MAC_servidor[2], MAC_servidor[3], MAC_servidor[4], MAC_servidor[5]);

  flashMemory.begin("model", false);
  String fecha_hora = flashMemory.getString("date_flash", "");
  flashMemory.end();

  json_doc["running"] = status_muestreo;
  json_doc["medidor_id"] = mac;
  json_doc["interval"] = second_muestreo;
  json_doc["update"] = fecha_hora;

  response->setCode(200);
  serializeJson(json_doc, *response);
  request->send(response);
}

void routeHandler_yuboxAPI_espnow_config_POST(AsyncWebServerRequest *request)
{
  YUBOX_RUN_AUTH(request);

  bool clientError = false;
  bool serverError = false;
  String responseMsg = "";
  AsyncWebParameter *p;

  int n_interval = 0;

  int n_MAC_1 = 0;
  int n_MAC_2 = 0;
  int n_MAC_3 = 0;
  int n_MAC_4 = 0;
  int n_MAC_5 = 0;
  int n_MAC_6 = 0;

#define ASSIGN_FROM_POST(TAG, FMT)                       \
  if (!clientError && request->hasParam(#TAG, true))     \
  {                                                      \
    p = request->getParam(#TAG, true);                   \
    int n = sscanf(p->value().c_str(), FMT, &(n_##TAG)); \
    if (n <= 0)                                          \
    {                                                    \
      clientError = true;                                \
      responseMsg = "Formato numérico incorrecto para "; \
      responseMsg += #TAG;                               \
    }                                                    \
  }

  ASSIGN_FROM_POST(MAC_1, "%li")
  ASSIGN_FROM_POST(MAC_2, "%li")
  ASSIGN_FROM_POST(MAC_3, "%li")
  ASSIGN_FROM_POST(MAC_4, "%li")
  ASSIGN_FROM_POST(MAC_5, "%li")
  ASSIGN_FROM_POST(MAC_6, "%li")
  ASSIGN_FROM_POST(interval, "%li")

  Serial.printf("%d:%d:%d:%d:%d:%d", n_MAC_1, n_MAC_2, n_MAC_3, n_MAC_4, n_MAC_5, n_MAC_6);

  if (!clientError && !serverError)
  {
    responseMsg = "Parámetros actualizados correctamente";

    second_muestreo = n_interval;
    MAC_servidor[0] = n_MAC_1;
    MAC_servidor[1] = n_MAC_2;
    MAC_servidor[2] = n_MAC_3;
    MAC_servidor[3] = n_MAC_4;
    MAC_servidor[4] = n_MAC_5;
    MAC_servidor[5] = n_MAC_6;

    flashMemory.begin(namespace_configuration, false);
    flashMemory.getInt(time_muestra, n_interval);
    flashMemory.putInt(addres_mac_1, n_MAC_1);
    flashMemory.putInt(addres_mac_2, n_MAC_2);
    flashMemory.putInt(addres_mac_3, n_MAC_3);
    flashMemory.putInt(addres_mac_4, n_MAC_4);
    flashMemory.putInt(addres_mac_5, n_MAC_5);
    flashMemory.putInt(addres_mac_6, n_MAC_6);
    flashMemory.end();

    if (n_interval > 0)
    {
      task_espnowMuestrearFases.disable();
      task_espnowMuestrearFases.setInterval(n_interval * TASK_SECOND);
      task_espnowMuestrearFases.enable();
    }
    else
    {
      task_espnowMuestrearFases.disable();
    }
  }

  unsigned int httpCode = 200;
  if (clientError)
    httpCode = 400;
  if (serverError)
    httpCode = 500;
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(httpCode);
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(2));
  json_doc["success"] = !(clientError || serverError);
  json_doc["msg"] = responseMsg.c_str();

  serializeJson(json_doc, *response);
  request->send(response);
}
