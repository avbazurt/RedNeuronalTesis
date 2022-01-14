#pragma once

#ifdef PIN_HARDWARE_SERIAL
#define ESP_serial Serial1
#else
#define ESP_serial Serial
#endif

#if CONFIG_IDF_TARGET_ESP32
// Pines de transmisión serial desde el punto de vista del ESP32
#define PIN_SERIAL_RX               GPIO_NUM_16
#define PIN_SERIAL_TX               GPIO_NUM_17

#elif CONFIG_IDF_TARGET_ESP32S2
// Pines de transmisión serial desde el punto de vista del ESP32
#define PIN_SERIAL_RX               GPIO_NUM_7
#define PIN_SERIAL_TX               GPIO_NUM_5

#else
#error Pines de control no definidos para board objetivo!
#endif
