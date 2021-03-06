/**
 * @file NexConfig.h
 *
 * Options for user can be found here. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/13
 * @author Jyrki Berg 2/17/2019 (https://github.com/jyberg)
 * 
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#ifndef __NEXCONFIG_H__
#define __NEXCONFIG_H__

/**
 * @addtogroup Configuration 
 * @{ 
 */

/**
 * Define STD_SUPPORT to enable c++ std templates usage like std::vector
 * 
*/
// #define STD_SUPPORT

/** 
 * Define NEX_DEBUG_SERIAL_ENABLE to enable debug serial. 
 * Comment it to disable debug serial. 
 */
// #define NEX_DEBUG_SERIAL_ENABLE

/**
 * Define dbSerial for the output of debug messages. 
 * it is resonsibility of main program to initialize debug serial port (begin(...)
 */
#ifdef NEX_DEBUG_SERIAL_ENABLE
#ifndef dbSerial
#define dbSerial Serial
#endif
#endif

/**
 * Define nexSerial for communicate with Nextion touch panel. 
 * Define NEX_SOFTWARE_SERIAL if software serial used
 * NodeMcu/Esp8266 can use harware serial (Serial) but it uses same serial as usb communication and
 * during SW upload NodeMcu RX pin must be disconnected from Nextion
 */
//#define NEX_SOFTWARE_SERIAL
#ifndef NEX_SOFTWARE_SERIAL

// hardware Serial port
#ifndef nexSerial
#ifdef ESP32
#define nexSerial Serial1
#else
#define nexSerial Serial
#endif
#endif

#else
// NodeMcu / Esp8266 Softwareserial if usb port used for debug 
// NodeMcu board pin numbers not match with Esp8266 pin numbers use NodeMcu Pin number definitions (pins_arduino.h)
#ifdef ESP32

// Ripped from HardwareSerial.cpp
#ifndef RX1
#if CONFIG_IDF_TARGET_ESP32
#define RX1 9
#elif CONFIG_IDF_TARGET_ESP32S2
#define RX1 18
#elif CONFIG_IDF_TARGET_ESP32C3
#define RX1 18
#endif
#endif

#ifndef TX1
#if CONFIG_IDF_TARGET_ESP32
#define TX1 10
#elif CONFIG_IDF_TARGET_ESP32S2
#define TX1 17
#elif CONFIG_IDF_TARGET_ESP32C3
#define TX1 19
#endif
#endif

#define NEX_RX RX1
#define NEX_TX TX1

#else

#define NEX_RX D2
#define NEX_TX D1

#endif  // #ifdef ESP32

#endif


#ifdef NEX_DEBUG_SERIAL_ENABLE
#define dbSerialPrint(a)    dbSerial.print(a)
#define dbSerialPrintln(a)  dbSerial.println(a)
#define dbSerialBegin(a)    dbSerial.begin(a)
#define dbSerialPrintByte(a) {if(a<10)dbSerial.print(0);dbSerial.print((unsigned char)a,HEX);}
#define dbSerialPrintlnByte(a) {if(a<10)dbSerial.print(0);dbSerial.println((unsigned char)a,HEX);}
#else
#define dbSerialPrint(a)   do{}while(0)
#define dbSerialPrintln(a) do{}while(0)
#define dbSerialBegin(a)   do{}while(0)
#define dbSerialPrintByte(a) do{}while(0)
#define dbSerialPrintlnByte(a) do{}while(0)
#endif

/**
 * @}
 */

#endif /* #ifndef __NEXCONFIG_H__ */
