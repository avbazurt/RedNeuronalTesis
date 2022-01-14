#include "HAL_now.h"
#include <ArduinoJson.h>
#include "HAL_config.h"

//#define PIN_HARDWARE_SERIAL 1

String inputString = "";      // Cadena para guardar el comando recibido
bool stringComplete = false;  // Bandera boleana que nos indica cuando el comando fue recibido y podemos compararlo con los 2 comandos válidos

ESP32_now *now;

#define RXD2 16
#define TXD2 17

long strHexDec(String text) {
  char msg[10];
  text.toCharArray(msg, 10);
  return (long)strtol(msg, 0, 16);
}

void Alarma(char MAC[], char text[])
{
  Serial.printf("Received message from: %s | %s\n", MAC, text);
}

void setup() {
  // initialize serial:
  Serial.begin(115200);
  #ifdef PIN_HARDWARE_SERIAL
    Serial.println("Pin Hardware Serial configurado");
    ESP_serial.begin(115200, SERIAL_8N1, PIN_SERIAL_RX, PIN_SERIAL_TX);
  #else
    Serial.println("Serial 0 Utilizado");
  #endif

  // reserve 200 bytes for the inputString:
  inputString.reserve(750);
  now = new ESP32_now();
  now->setReciveCallback(Alarma);
  now->begin();
}

void loop() {
  if (stringComplete) {//El comando fue recibido, procedemos a compararlo
    Serial.println(inputString);
    DynamicJsonDocument doc(2024);
    deserializeJson(doc, inputString);

    const char* cmd = doc["cmd"];

    if (String(cmd) == "now") {
      const char* msg = doc["msg"];
      const char* MAC = doc["MAC"];

      String str_mac = String(MAC);

      if (str_mac.length() == 17) {
        int mac1 = strHexDec(str_mac.substring(0, 2));
        int mac2 = strHexDec(str_mac.substring(3, 5));
        int mac3 = strHexDec(str_mac.substring(6, 8));
        int mac4 = strHexDec(str_mac.substring(9, 11));
        int mac5 = strHexDec(str_mac.substring(12, 14));
        int mac6 = strHexDec(str_mac.substring(15, 17));

        uint8_t peerAddress[] = {mac1, mac2, mac3, mac4, mac5, mac6 };

        String text = String(msg);

        Serial.print("msg: ");
        Serial.println(text);
        now->sentData(peerAddress, msg);        
      }
      else {
        //Serial.println("MAC invalida");
        2+2;
      }
    }

    inputString = "";//Limpiamos la cadena para poder recibir el siguiente comando
    stringComplete = false;//Bajamos la bandera para no volver a ingresar a la comparación hasta que recibamos un nuevo comando
  }
}



#ifdef PIN_HARDWARE_SERIAL
void serialEvent1() 
#else 
void serialEvent()
#endif
{
  while (ESP_serial.available()) {//Mientras tengamos caracteres disponibles en el buffer
    char inChar = (char)ESP_serial.read();//Leemos el siguiente caracter
    if (inChar == '\n') {//Si el caracter recibido corresponde a un salto de línea
      stringComplete = true;//Levantamos la bandera
    }
    else { //Si el caracter recibido no corresponde a un salto de línea
      inputString += inChar;//Agregamos el caracter a la cadena
    }
  }
}
