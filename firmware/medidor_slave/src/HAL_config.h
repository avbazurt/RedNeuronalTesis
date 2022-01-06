#pragma once

// Pines a usar para Sensor
#define PZEM_RX_PIN                 GPIO_NUM_16
#define PZEM_TX_PIN                 GPIO_NUM_17
#define PZEM_SERIAL                 Serial2

// Pines de transmisión serial desde el punto de vista del ESP32
#define NEXTION_TX_PIN              GPIO_NUM_19
#define NEXTION_RX_PIN              GPIO_NUM_18
#define NEXTION_SERIAL              Serial1