#include <Arduino.h>
#include "NeuralNetwork.h"
#include "HAL_now.h"
#include "PZEM_Trifasico.h"
#include "HAL_NextionUI.h"
#include "HAL_config.h"
#include <TaskScheduler.h>
#include <WiFi.h>
#include <YuboxSimple.h>

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

// Callback
void ReciveDataNow(char MAC[], char text[]);

void setup()
{
  NextionUI_initialize();
  Serial.begin(115200);
  sensor = new PZEM_Trifasico(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN,
                              PZEM_ADDRESS_FASE_A,
                              PZEM_ADDRESS_FASE_B,
                              PZEM_ADDRESS_FASE_C);

  nn = new NeuralNetwork();

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
  sensor->GetMedicionTrifasica();
  float number1 = 5.01 * (random(100) / 100.0);
  float number2 = 5.01 * (random(100) / 100.0);
  nn->getInputBuffer()[0] = number1;
  nn->getInputBuffer()[1] = number2;
  float result = nn->predict();

  Serial.printf("%.2f %.2f - result %.2f \n", number1, number2, result);
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