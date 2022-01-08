#pragma once

// Direcciones Sensores
#define PZEM_ADDRESS_FASE_A         0x00
#define PZEM_ADDRESS_FASE_B         0x01
#define PZEM_ADDRESS_FASE_C         0x02

// Pines de transmison serial PZEM desde el punto de vista del ESP32
#define PZEM_RX_PIN                 GPIO_NUM_16
#define PZEM_TX_PIN                 GPIO_NUM_17
#define PZEM_SERIAL                 Serial2

// Pines de transmisión serial Nextion desde el punto de vista del ESP32
#define NEXTION_SERIAL_TX           GPIO_NUM_26
#define NEXTION_SERIAL_RX           GPIO_NUM_25
