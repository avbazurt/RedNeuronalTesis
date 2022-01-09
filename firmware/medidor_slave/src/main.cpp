#include <Arduino.h>
#include "NeuralNetwork.h"
#include "PZEM_Trifasico.h"
#include <YuboxSimple.h>

#include <TaskScheduler.h>
#include <RTClib.h>

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

// Clases Necesarias
ESP32_now *now;
NeuralNetwork *nn;
PZEM_Trifasico *sensor;

// Objeto que controla el RTC DS3231
RTC_DS3231 rtc;
void iniciarHoraDesdeRTC(void);


// Callback
void ReciveDataNow(char MAC[], char text[]);

void setup()
{
  NextionUI_initialize(yubox_HTTPServer);
  Serial.begin(115200);
  iniciarHoraDesdeRTC();

  sensor = new PZEM_Trifasico(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN,
                              PZEM_ADDRESS_FASE_A,
                              PZEM_ADDRESS_FASE_B,
                              PZEM_ADDRESS_FASE_C);

  sensor->setModeSimulation();

  nn = new NeuralNetwork();

  WiFi.mode(WIFI_AP_STA);
  yuboxSimpleSetup();
  yuboxAddManagedHandler(&eventosLector);
  YuboxWiFi.releaseControlOfWiFi();
  YuboxWiFi.saveControlOfWiFi();

  now = new ESP32_now();
  now->setReciveCallback(ReciveDataNow);
  now->begin();

  task_predition.enable();
  task_yuboxMuestrearFases.enable();
}

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
  DateTime now = rtc.now();

  // Calcular milisegundos desde la medianoche para la hora actual y para cada
  // uno de los instantes de alimentación.
#define SEC_MEDIANOCHE(HH, MM, SS) ((SS + 60 * (MM + 60 * HH)))
#define SEC_EN_DIA 86400000

  uint32_t sec_ahora = SEC_MEDIANOCHE(now.hour(), now.minute(), now.second());

  //Serial.printf("hora actual es: %02d:%02d:%02d (%d ms desde medianoche)\n",
  //              now.hour(), now.minute(), now.second(), msec_ahora);

  float seconds = sec_ahora;
  float day = now.dayOfTheWeek();

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
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!rtc.begin(&Wire))
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
  }
}