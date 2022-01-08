#include <Arduino.h>
#include "NeuralNetwork.h"
#include "HAL_now.h"
#include "PZEM_Trifasico.h"
#include <ArduinoJson.h>
#include "HAL_NextionUI.h"
#include "HAL_config.h"

ESP32_now *now;
NeuralNetwork *nn;
PZEM_Trifasico *sensor;

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
    NextionUI_flah_model(doc["len"], true);
    delay(100);
    nn->len_new_model_tflite = doc["len"];
    Serial.println("Creando el Array");
    nn->new_model_tflite = new char[nn->len_new_model_tflite];
  }

  else if (String(opcion) == "model")
  {
    const char *valor = doc["valor"];
    int indice = doc["indice"];

    int valor_entero = (long)strtol(valor, 0, 16);
    nn->new_model_tflite[indice] = valor_entero;
    Serial.println(nn->new_model_tflite[indice], HEX);
    NextionUI_NextIndice(indice);
  }

  else
  {
    Serial.println("Flasheando modelo");
    nn->SaveModel();
    ESP.restart();
  }
}

void setup()
{
  NextionUI_initialize();
  Serial.begin(115200);

  sensor = new PZEM_Trifasico(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN,
                              PZEM_ADDRESS_FASE_A,
                              PZEM_ADDRESS_FASE_B,
                              PZEM_ADDRESS_FASE_C);

  nn = new NeuralNetwork();
  now = new ESP32_now();

  now->setReciveCallback(ReciveDataNow);
  now->begin();
}

void loop()
{
  /*
  sensor->GetMedicionTrifasica();
  float number1 = 5.01 * (random(100) / 100.0);
  float number2 = 5.01 * (random(100) / 100.0);

  nn->getInputBuffer()[0] = number1;
  nn->getInputBuffer()[1] = number2;

  float result = nn->predict();

  Serial.printf("%.2f %.2f - result %.2f \n", number1, number2, result);
  delay(500);
  */
  NextionUI_runEvents(*sensor);
}