/**
 * @file NexHardware.cpp
 *
 * The implementation of base API for using Nextion. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/11
 * @author Jyrki Berg 2/17/2019 (https://github.com/jyberg)
 * 
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include "NexHardware.h"


#ifdef NEX_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
SoftwareSerial nexSerial(NEX_RX,NEX_TX);
#endif

#define NEX_RET_EVENT_NEXTION_STARTUP       (0x00)  //Returned when Nextion has started or reset
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)  //Returned when Touch occurs and component’s corresponding Send Component ID is checked in the users HMI design.
#define NEX_RET_CURRENT_PAGE_ID_HEAD        (0x66)  //Returned when the sendme command is used.
#define NEX_RET_EVENT_POSITION_HEAD         (0x67)  //Returned when sendxy=1 and not in sleep mode
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD   (0x68)  //Returned when sendxy=1 and in sleep mode
#define NEX_RET_STRING_HEAD                 (0x70)  //Returned when using get command for a string.
#define NEX_RET_NUMBER_HEAD                 (0x71)  //Returned when get command to return a number 4 byte 32-bit value in little endian order.
#define NEX_RET_AUTOMATIC_SLEEP             (0x86)  //Returned when Nextion enters sleep automatically Using sleep=1 will not return an 0x86
#define NEX_RET_AUTOMATIC_WAKE_UP           (0x87)  //Returned when Nextion leaves sleep automatically Using sleep=0 will not return an 0x87
#define NEX_RET_EVENT_NEXTION_READY         (0x88)  //Returned when Nextion has powered up and is now initialized successfully
#define NEX_RET_START_SD_UPGRADE            (0x89)  //Returned when power on detects inserted microSD and begins Upgrade by microSD process
#define Nex_RET_TRANSPARENT_DATA_FINISHED   (0xFD)  //Returned when all requested bytes of Transparent Data mode have been received, and is now leaving transparent data mode
#define Nex_RET_TRANSPARENT_DATA_READY      (0xFE)  //Returned when requesting Transparent Data mode, and device is now ready to begin receiving the specified quantity of data

#define NEX_RET_INVALID_CMD             (0x00)  //Returned when instruction sent by user has failed
#define NEX_RET_CMD_FINISHED_OK         (0x01)  //Returned when instruction sent by user was successful
#define NEX_RET_INVALID_COMPONENT_ID    (0x02)  //Returned when invalid Component ID or name was used
#define NEX_RET_INVALID_PAGE_ID         (0x03)  //Returned when invalid Page ID or name was used
#define NEX_RET_INVALID_PICTURE_ID      (0x04)  //Returned when invalid Picture ID was used
#define NEX_RET_INVALID_FONT_ID         (0x05)  //Returned when invalid Font ID was used
#define NEX_RET_INVALID_FILE_OPERATION  (0x06)  //Returned when File operation fails
#define NEX_RET_INVALID_CRC             (0x09)  //Returned when Instructions with CRC validation fails their CRC check
#define NEX_RET_INVALID_BAUD            (0x11)  //Returned when invalid Baud rate was used
#define NEX_RET_INVALID_WAVEFORM_ID_OR_CHANNEL_NRO  (0x12)  //Returned when invalid Waveform ID or Channel # was used
#define NEX_RET_INVALID_VARIABLE_OR_ATTRIBUTE       (0x1A)  //Returned when invalid Variable name or invalid attribute was used
#define NEX_RET_INVALID_VARIABLE_OPERATION          (0x1B)  //Returned when Operation of Variable is invalid. ie: Text assignment t0.txt=abc or t0.txt=23, Numeric assignment j0.val=”50″ or j0.val=abc
#define NEX_RET_ASSIGNMENT_FAILED_TO_ASSIGN         (0x1C)  //Returned when attribute assignment failed to assign
#define NEX_RET_EEPROM_OPERATION_FAILED             (0x1D)  //Returned when an EEPROM Operation has failed
#define NEX_RET_INVALID_QUANTITY_OF_PARAMETERS      (0x1E)  //Returned when the number of instruction parameters is invalid
#define NEX_RET_IO_OPERATION_FAILED                 (0x1F)  //Returned when an IO operation has failed
#define NEX_RET_ESCAPE_CHARACTER_INVALID            (0x20)  //Returned when an unsupported escape character is used
#define NEX_RET_VARIABLE_NAME_TOO_LONG              (0x23)  //Returned when variable name is too long. Max length is 29 characters: 14 for page + “.” + 14 for component.
#define NEX_RET_SERIAL_BUFFER_OVERFLOW              (0x24)  //Returned when a Serial Buffer overflow occurs Buffer will continue to receive the current instruction, all previous instructions are lost.

void (*nextionStartupCallback)() =nullptr;
void (*currentPageIdCallback)(uint8_t) =nullptr;
void (*touchCoordinateCallback)(uint16_t,uint16_t,uint8_t)=nullptr;
void(*touchEventInSleepModeCallback)(uint16_t,uint16_t,uint8_t) =nullptr;
void (*automaticSleepCallback)() =nullptr;
void (*automaticWakeUpCallback)() =nullptr;
void (*nextionReadyCallback)() =nullptr;
void (*startSdUpgradeCallback)() =nullptr;

static uint8_t _nextion_event_size[][2] = {
    {NEX_RET_EVENT_NEXTION_STARTUP,     6},
    {NEX_RET_EVENT_TOUCH_HEAD,          7},
    {NEX_RET_CURRENT_PAGE_ID_HEAD,      5},
    {NEX_RET_EVENT_POSITION_HEAD,       9},
    {NEX_RET_EVENT_SLEEP_POSITION_HEAD, 9},
    {NEX_RET_AUTOMATIC_SLEEP,           4},
    {NEX_RET_AUTOMATIC_WAKE_UP,         4},
    {NEX_RET_EVENT_NEXTION_READY,       1},
    {NEX_RET_START_SD_UPGRADE,          1},
    {0xFF,                              0}  // <-- fence, do not remove
};

/* Simple linked list queue of received but undelivered touch event packets */
struct _nextion_touch_event
{
       struct _nextion_touch_event * next;
       uint8_t event_data[10];
};
static struct _nextion_touch_event * _first = NULL;
static struct _nextion_touch_event * _last = NULL;

/* Push event data to back of queue */
static int16_t _pushEventData(uint8_t c)
{
    unsigned int i;

    do {
        // Look up expected event size, return byte if unknown event
        i = 0;
        while (_nextion_event_size[i][1] != 0) {
            if (_nextion_event_size[i][0] == c) break;
            i++;
        }
        if (!_nextion_event_size[i][1]) return c;
        uint8_t n = _nextion_event_size[i][1];

        uint32_t t = millis();
        while (nexSerial.available() < (n - 1) && millis() - t < 200) {
            delay(1);
        }
        if (nexSerial.available() < (n - 1)) return c;

        // New event packet, byte
        struct _nextion_touch_event * e = new struct _nextion_touch_event;
        e->next = NULL;
        e->event_data[0] = c;
        for (i = 1; i < n; i++) e->event_data[i] = nexSerial.read();

        // Add to linked list
        if (_last != NULL) _last->next = e;
        _last = e;
        if (_first == NULL) _first = e;

        if (!nexSerial.available()) return -1;
        c = nexSerial.read();
    } while (true);
}

/* Remove event data from front of queue */
static void _popEventData()
{
    struct _nextion_touch_event * e = _first;
    if (e == NULL) return;
    _first = _first->next;
    delete e;
    if (_first == NULL) _last = NULL;
}

/*
 * Receive unt32_t data. 
 * 
 * @param number - save uint32_t data. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.
 *
 */
bool recvRetNumber(uint32_t *number, size_t timeout)
{
    bool ret = false;
    uint8_t temp[8] = {0};

    if (!number)
    {
        goto __return;
    }
    
    if (sizeof(temp) != readBytes(temp, sizeof(temp), timeout))
    {
        goto __return;
    }

    if (temp[0] == NEX_RET_NUMBER_HEAD
        && temp[5] == 0xFF
        && temp[6] == 0xFF
        && temp[7] == 0xFF
        )
    {
        *number = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | ((uint32_t)temp[2] << 8) | (temp[1]);
        ret = true;
    }

__return:

    if (ret) 
    {
        dbSerialPrint("recvRetNumber :");
        dbSerialPrintln(*number);
    }
    else
    {
        dbSerialPrintln("recvRetNumber err");
    }
    
    return ret;
}

/*
 * Receive int32_t data. 
 * 
 * @param number - save int32_t data. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.
 *
 */
bool recvRetNumber(int32_t *number, size_t timeout)
{
    bool ret = false;
    uint8_t temp[8] = {0};

    if (!number)
    {
        goto __return;
    }
    
    if (sizeof(temp) != readBytes(temp, sizeof(temp),timeout))
    {
        goto __return;
    }

    if (temp[0] == NEX_RET_NUMBER_HEAD
        && temp[5] == 0xFF
        && temp[6] == 0xFF
        && temp[7] == 0xFF
        )
    {
        *number = ((int32_t)temp[4] << 24) | ((int32_t)temp[3] << 16) | ((int32_t)temp[2] << 8) | (temp[1]);
        ret = true;
    }

__return:

    if (ret) 
    {
        dbSerialPrint("recvRetNumber :");
        dbSerialPrintln(*number);
    }
    else
    {
        dbSerialPrintln("recvRetNumber err");
    }
    
    return ret;
}

/*
 * Receive string data. 
 * 
 * @param str - save string data. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.
 *
 */
bool recvRetString(String &str, size_t timeout)
{
    str = "";
    bool ret{false};
    bool str_start_flag{false};
    uint8_t cnt_0xff = 0;
    uint8_t c = 0;
    uint32_t start{millis()};
    size_t avail{(size_t)nexSerial.available()};
    while(ret == false && (millis()-start)<timeout)
    {
        while (nexSerial.available())
        {
            c = nexSerial.read();
            dbSerialPrint(c);

            if (str_start_flag)
            {
                if (0xFF == c)
                {
                    cnt_0xff++;                    
                    if (cnt_0xff >= 3)
                    {
                        ret = true;
                        break;
                    }
                }
                else
                {
                    str += (char)c;
                }
            }
            else if (NEX_RET_STRING_HEAD == c)
            {
                str_start_flag = true;
            }
            yield();
        }
        delayMicroseconds(20);
    }
    dbSerialPrintln("");
    dbSerialPrint("recvRetString[");
    dbSerialPrint(str.length());
    dbSerialPrint(",");
    dbSerialPrint(str);
    dbSerialPrintln("]");

    return ret;
}

/*
 * Receive string data. 
 * 
 * @param buffer - save string data. 
 * @param len - in buffer len / out saved string len excluding null char. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.  
 *
 */
bool recvRetString(char *buffer, uint16_t &len, size_t timeout)
{
    String temp;
    bool ret = recvRetString(temp,timeout);

    if(ret && len)
    {
        len=temp.length()>len?len:temp.length();
        strncpy(buffer,temp.c_str(), len);
    }
    return ret;
}

/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void sendCommand(const char* cmd)
{
    // empty in buffer for clean responce
    while (nexSerial.available())
    {
        //nexSerial.read();

        int16_t c = _pushEventData(nexSerial.read());
        if (c != -1) {
            // Skip whatever it is that remains in the buffer by looking for 
            // end-of-packet marker
            //Serial.printf("| %02x", c);
            auto n = 3;
            while (n > 0) {
                c = nexSerial.read();
                //Serial.printf(" %02x", c);
                if (c == -1) break;
                if (c == 0xff) n--; else n = 3;
            }
        }

    }

    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    dbSerialPrintln(cmd);
}

#ifdef STD_SUPPORT
void sendRawData(const std::vector<uint8_t> &data)
{
    nexSerial.write(data.data(),data.size());
}
#endif

void sendRawData(const uint8_t *buf, uint16_t len)
{
    nexSerial.write(buf, len);
}

void sendRawByte(const uint8_t byte)
{
    nexSerial.write(&byte, 1);
}

size_t readBytes(uint8_t* buffer, size_t size, size_t timeout)
{
    uint32_t start{millis()};
    size_t avail;
    bool gotByte = false;

    while(!gotByte && (millis()-start)<timeout) {
        avail=nexSerial.available();
        if (avail > 0) {
            int16_t c = _pushEventData(nexSerial.read());
            if (c != -1) {
                gotByte = true;
                *buffer = c;
                ++buffer;
                size--;
            }
            avail=nexSerial.available();
        } else {
            // Ensure vTaskDelay is called on ESP32
            delay(1);
        }
    }

    while(size>avail && (millis()-start)<timeout)
    {
        avail=nexSerial.available();
        if (!avail) delay(1);
    }
    size_t read=min(size,avail);
    for(size_t i{read}; i;--i)
    {
        *buffer=nexSerial.read();
        ++buffer;
    }
    return read + (gotByte ? 1 : 0);
}

bool recvCommand(const uint8_t command, size_t timeout)
{
    bool ret = false;
    uint8_t temp[4] = {0};
    size_t rd;

    rd = readBytes((uint8_t*)&temp, sizeof(temp), timeout);
    if (sizeof(temp) != rd)
    {
#ifdef dbSerial
        dbSerial.printf("recv command timeout: got %d bytes, expected %d\r\n",
            rd, sizeof(temp));
#endif
        ret = false;
    }
    else
    {
        if (temp[0] == command
            && temp[1] == 0xFF
            && temp[2] == 0xFF
            && temp[3] == 0xFF
            )
        {
            ret = true;
        }
        else
        {
            dbSerialPrint("recv command err value: ");
            dbSerialPrintlnByte(temp[0]);  
        }
    }
    return ret;
}

bool recvRetCommandFinished(size_t timeout)
{
    bool ret = recvCommand(NEX_RET_CMD_FINISHED_OK, timeout);
    if (ret) 
    {
        dbSerialPrintln("recvRetCommandFinished ok");
    }
    else
    {
        dbSerialPrintln("recvRetCommandFinished err");
    }
    return ret;
}

bool RecvTransparendDataModeReady(size_t timeout)
{
    dbSerialPrintln("RecvTransparendDataModeReady requested");
    bool ret = recvCommand(Nex_RET_TRANSPARENT_DATA_READY, timeout);
    if (ret) 
    {
        dbSerialPrintln("RecvTransparendDataModeReady ok");
    }
    else
    {
        dbSerialPrintln("RecvTransparendDataModeReady err");
    }
    return ret;
}

bool RecvTransparendDataModeFinished(size_t timeout)
{
    bool ret = recvCommand(Nex_RET_TRANSPARENT_DATA_FINISHED, timeout);
    if (ret) 
    {
        dbSerialPrintln("RecvTransparendDataModeFinished ok");
    }
    else
    {
        dbSerialPrintln("RecvTransparendDataModeFinished err");
    }
    return ret;
}


bool nexInit(const uint32_t baud)
{
    bool ret1 = false;
    bool ret2 = false;

    if (baud > 0) {
        nexSerial.begin(9600); // default baud, it is recommended that do not change defaul baud on Nextion, because it can forgot it on re-start
        if(baud!=9600)
        {
            char cmd[14];
            sprintf(cmd,"baud=%i",baud);
            sendCommand(cmd);
            delay(100);
            nexSerial.begin(baud);
            delay(20);
        }
    }

    sendCommand("bkcmd=3");
    ret1 = recvRetCommandFinished();
    sendCommand("page 0");
    ret2 = recvRetCommandFinished();
    sendCommand("connect");
    String str;
    recvRetString(str,1000);
    dbSerialPrintln(str);

    return ret2;
}

void nexLoop(NexTouch *nex_listen_list[])
{
    while (_first != NULL || nexSerial.available())
    {
        int16_t c = -1;

        if (_first == NULL) c = _pushEventData(nexSerial.read());

        if (_first != NULL) {
            switch(_first->event_data[0]) {
            case NEX_RET_EVENT_NEXTION_STARTUP:
            {
                if (0x00 == _first->event_data[1] && 0x00 == _first->event_data[2] && 0xFF == _first->event_data[3] && 0xFF == _first->event_data[4] && 0xFF == _first->event_data[5])
                {
                    if(nextionStartupCallback!=nullptr)
                    {
                        nextionStartupCallback();
                    }
                }
                break;
            }
            case NEX_RET_EVENT_TOUCH_HEAD:
            {
                if (0xFF == _first->event_data[4] && 0xFF == _first->event_data[5] && 0xFF == _first->event_data[6])
                {
                    NexTouch::iterate(nex_listen_list, _first->event_data[1], _first->event_data[2], _first->event_data[3]);
                }
                break;
            }
            case NEX_RET_CURRENT_PAGE_ID_HEAD:
            {
                if (0xFF == _first->event_data[2] && 0xFF == _first->event_data[3] && 0xFF == _first->event_data[4])
                {
                    if(currentPageIdCallback!=nullptr)
                    {
                        currentPageIdCallback(_first->event_data[1]);
                    }
                }
                break;
            }
            case NEX_RET_EVENT_POSITION_HEAD:
            case NEX_RET_EVENT_SLEEP_POSITION_HEAD:
            {
                if (0xFF == _first->event_data[6] && 0xFF == _first->event_data[7] && 0xFF == _first->event_data[8])
                {
                    if(_first->event_data[0] == NEX_RET_EVENT_POSITION_HEAD && touchCoordinateCallback!=nullptr)
                    {
                        touchCoordinateCallback(((int16_t)_first->event_data[2] << 8) | (_first->event_data[1]), ((int16_t)_first->event_data[4] << 8) | (_first->event_data[3]),_first->event_data[5]);
                    }
                    else if(_first->event_data[0] == NEX_RET_EVENT_SLEEP_POSITION_HEAD && touchCoordinateCallback!=nullptr)
                    {
                        touchEventInSleepModeCallback(((int16_t)_first->event_data[2] << 8) | (_first->event_data[1]), ((int16_t)_first->event_data[4] << 8) | (_first->event_data[3]),_first->event_data[5]);
                    }
                }
                break;
            }
            case NEX_RET_AUTOMATIC_SLEEP:
            case NEX_RET_AUTOMATIC_WAKE_UP:
            {
                if (0xFF == _first->event_data[1] && 0xFF == _first->event_data[2] && 0xFF == _first->event_data[3])
                {
                    if(_first->event_data[0]==NEX_RET_AUTOMATIC_SLEEP && automaticSleepCallback!=nullptr)
                    {
                        automaticSleepCallback();
                    }
                    else if(_first->event_data[0]==NEX_RET_AUTOMATIC_WAKE_UP && automaticWakeUpCallback!=nullptr)
                    {
                        automaticWakeUpCallback();
                    }
                }
                break;
            }
            case NEX_RET_EVENT_NEXTION_READY:
            {
                if(nextionReadyCallback!=nullptr)
                {
                    nextionReadyCallback();
                }
                break;
            }
            case NEX_RET_START_SD_UPGRADE:
            {
                if(startSdUpgradeCallback!=nullptr)
                {
                    startSdUpgradeCallback();
                }
                break;
            }
            };

            _popEventData();
        } else if (c != -1) {
            // unnoun data clean buffer.
            dbSerialPrint("Unexpected data received hex: [");
            do {
                dbSerialPrintByte((uint8_t)c);
                c = nexSerial.available() ? nexSerial.read() : -1;
            } while (c != -1);
            dbSerialPrintln("]");
        }
    }
}
