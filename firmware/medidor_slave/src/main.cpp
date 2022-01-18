#include <Arduino.h>
#include "NeuralNetwork.h"
#include "PZEM_Trifasico.h"
#include <YuboxSimple.h>
#include <Preferences.h>

#include <TaskScheduler.h>
#include <RTClib.h>
#include "PCA9536.h"

#include "HAL_now.h"
#include "HAL_NextionUI.h"
#include "HAL_config.h"

// Definicion YUBOX FRAMEWORK
#define ARDUINOJSON_USE_LONG_LONG 1
Scheduler mainScheduler;
AsyncEventSource eventosLector("/yubox-api/lectura/events");

// Tareas
void yuboxMuestrearFases(void);
Task task_yuboxMuestrearFases(TASK_SECOND * 1, TASK_FOREVER, &yuboxMuestrearFases, &mainScheduler);

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

float adjuste_FaseA = 0.5;
float adjuste_FaseB = 0.5;
float adjuste_FaseC = 0.5;


// Objeto que controla el RTC DS3231
RTC_DS3231 rtc;
bool rtc_valid = false;
void iniciarHoraDesdeRTC(void);

// Callback
void ReciveDataNow(char MAC[], char text[]);
int histeresis(float valor, float tp);

void setup()
{
  NextionUI_initialize(yubox_HTTPServer);
  Serial.begin(115200);
  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);

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
}

uint32_t port = 0;

void loop()
{
  yuboxSimpleLoopTask();
  NextionUI_runEvents(*sensor);
  mainScheduler.execute();
}

void yuboxMuestrearFases(void)
{
  DynamicJsonDocument json_doc(JSON_OBJECT_SIZE(3));
  json_doc["ts"] = 1000ULL * YuboxNTPConf.getUTCTime();
  json_doc["temperature"] = random(1, 100);
  json_doc["pressure"] = random(1, 100);

  String json_output;
  serializeJson(json_doc, json_output);

  if (eventosLector.count() > 0)
  {
    eventosLector.send(json_output.c_str());
  }
}

void TaskPredition()
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
  float day = timeinfo.tm_wday;

  sensor->GetMedicionTrifasica();

  nn->getInputBuffer()[0] = seconds;
  nn->getInputBuffer()[1] = day;
  nn->getInputBuffer()[2] = sensor->DatosFaseA.FP;
  nn->getInputBuffer()[3] = sensor->DatosFaseB.FP;
  nn->getInputBuffer()[4] = sensor->DatosFaseC.FP;
  nn->getInputBuffer()[5] = sensor->DatosFaseA.P;
  nn->getInputBuffer()[6] = sensor->DatosFaseB.P;
  nn->getInputBuffer()[7] = sensor->DatosFaseC.P;

  float *result = nn->getOutputBuffer();

  Serial.printf("%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f- result %.2f %.2f %.2f\n",
                seconds, day, sensor->DatosFaseA.FP, sensor->DatosFaseB.FP, sensor->DatosFaseC.FP, sensor->DatosFaseA.P, sensor->DatosFaseB.P, sensor->DatosFaseC.P, result[0], result[1], result[2]);


  int rele_faseA = 0b00000001*((int)(result[0]>=adjuste_FaseA));
  int rele_faseB = 0b00000010*((int)(result[1]>=adjuste_FaseB));
  int rele_faseC = 0b00000100*((int)(result[2]>=adjuste_FaseC));

  int output_rele = rele_faseA | rele_faseB | rele_faseC;
  Serial.println(output_rele,BIN);

  /*
  int rele_faseA = histeresis(result[0], 0.30);
  int rele_faseB = histeresis(result[1], 0.40);
  int rele_faseC = histeresis(result[2], 0.30);



  Serial.printf("hora actual es: %02d:%02d:%02d \n", now.hour(), now.minute(), now.second());

  Serial.printf("%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f- result %.2f %.2f %.2f\n",
                seconds, day, sensor->DatosFaseA.FP, sensor->DatosFaseB.FP, sensor->DatosFaseC.FP, sensor->DatosFaseA.P, sensor->DatosFaseB.P, sensor->DatosFaseC.P, result[0], result[1], result[2]);


  Serial.println("Histeresis: ");
  Serial.println(rele_faseA);
  Serial.println(rele_faseB);
  Serial.println(rele_faseC);

  int output_rele = 0b00000000;
  Serial.println("Control Reles");
  Serial.println(output_rele, BIN);

  output_rele = output_rele | (rele_faseA * 0b00000001);
  Serial.println(output_rele, BIN);
  output_rele = output_rele | (rele_faseB * 0b00000010);
  Serial.println(output_rele, BIN);
  output_rele = output_rele | (rele_faseC * 0b00000100);
  Serial.println(output_rele, BIN);
  */
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

      Preferences flashMemory;
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

int histeresis(float valor, float tp)
{
  if (valor > tp)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void mostrarHora(void)
{
  struct tm timeinfo;
  time_t ts_ahora;

  // ¿Qué hora es? Se asume hora sistema correcta vía NTP
  ts_ahora = time(NULL);
  localtime_r(&ts_ahora, &timeinfo);

  // Calcular milisegundos desde la medianoche para la hora actual y para cada
  // uno de los instantes de alimentación.
#define MSEC_MEDIANOCHE(HH, MM, SS) (1000 * (SS + 60 * (MM + 60 * HH)))
#define MSEC_EN_DIA 86400000

  uint32_t msec_ahora = MSEC_MEDIANOCHE(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  Serial.printf("hora actual es: %02d:%02d:%02d (%d ms desde medianoche)\n",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, msec_ahora);
}