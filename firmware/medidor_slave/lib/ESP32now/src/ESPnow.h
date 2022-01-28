#pragma once
#include "Arduino.h"
#include <esp_now.h>


class ESP32_now
{
private:
    static void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
    static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
    static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
    
public:
    esp_now_peer_info_t *dispositivos;


    String nameGroup;



    ESP32_now(String name = "ESP");
    void begin();

    static void setReciveCallback(void (*puntero)(char MAC[], char text[])); 
    bool sentData(uint8_t peerAddress[], const String &message, bool validate_send = false);

    void ScanForSlave();

};