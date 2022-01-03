#include <Arduino.h>
#include "NeuralNetwork.h"
#include "now.h"
#include <ArduinoJson.h>
#include <YuboxSimple.h>

#define ARDUINOJSON_USE_LONG_LONG 1

ESP32_now *now;
NeuralNetwork *nn;

void Alarma(char MAC[], char text[])
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

  if (String(opcion) == "model")
  {
    Serial.println("Hola");
  }
}

void setup()
{
  Serial.begin(115200);
  nn = new NeuralNetwork();
  now = new ESP32_now();

  yuboxSimpleSetup();

  now->setReciveCallback(Alarma);
  now->begin();
}

void loop()
{

  /*
  float number1 = 5.01 * (random(100) / 100.0);
  float number2 = 5.01 * (random(100) / 100.0);

  nn->getInputBuffer()[0] = number1;
  nn->getInputBuffer()[1] = number2;

  float result = nn->predict();

  const char *expected = number2 > number1 ? "True" : "False";

  const char *predicted = result > 0.5 ? "True" : "False";

  Serial.printf("%.2f %.2f - result %.2f - Expected %s, Predicted %s\n", number1, number2, result, expected, predicted);

  delay(1000);
  */
}