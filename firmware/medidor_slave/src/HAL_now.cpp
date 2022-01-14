#include "Arduino.h"
#include "HAL_now.h"
#include <WiFi.h>
#include <ArduinoJson.h>

void (*punteroCallback)(char MAC[], char text[]);
bool status_send = false;
bool init_now = false;

void ESP32_now::formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void ESP32_now::receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{

    // only allow a maximum of 250 characters in the message + a null terminating byte
    char buffer[ESP_NOW_MAX_DATA_LEN + 1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    strncpy(buffer, (const char *)data, msgLen);
    // make sure we are null terminated
    buffer[msgLen] = 0;
    // format the mac address
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    // debug log the message to the serial port
    punteroCallback(macStr, buffer);
}

void ESP32_now::sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{

    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    Serial.print("Last Packet Sent to: ");
    Serial.println(macStr);
    Serial.print("Last Packet Send Status: ");

    if (status == ESP_NOW_SEND_SUCCESS)
    {
        status_send = true;
    }
    else
    {
        status_send = false;
    }

    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    //return status == ESP_NOW_SEND_SUCCESS;
}

void ESP32_now::setReciveCallback(void (*puntero)(char MAC[], char text[]))
{
    punteroCallback = puntero;
}

ESP32_now::ESP32_now()
{
    WiFi.mode(WIFI_AP_STA);
    Serial.println("ESPNow Example");
    // Output my MAC address - useful for later
    Serial.print("My MAC Address is: ");
    Serial.println(WiFi.macAddress());
    // shut down wifi
    //WiFi.disconnect();

    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success");
    }
    else
    {
        Serial.println("ESPNow Init Failed");
        delay(3000);
        ESP.restart();
    }
    init_now = true;
}

void ESP32_now::begin()
{
    esp_now_register_send_cb(sentCallback);
    esp_now_register_recv_cb(receiveCallback);
}

bool ESP32_now::sentData(uint8_t peerAddress[], const String &message)
{
    // and this will send a message to a specific device
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, peerAddress, 6);
    if (!esp_now_is_peer_exist(peerAddress))
    {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());

    Serial.print("Estado: ");
    Serial.println(status_send);

    return status_send;
    /*
    if (result == ESP_OK)
    {
        Serial.println("Broadcast message success");
        Serial.println("");

        return true;
    }
    else
    {
        if (result == ESP_ERR_ESPNOW_NOT_INIT)
        {
            Serial.println("ESPNOW not Init.");
        }
        else if (result == ESP_ERR_ESPNOW_ARG)
        {
            Serial.println("Invalid Argument");
        }
        else if (result == ESP_ERR_ESPNOW_INTERNAL)
        {
            Serial.println("Internal Error");
        }
        else if (result == ESP_ERR_ESPNOW_NO_MEM)
        {
            Serial.println("ESP_ERR_ESPNOW_NO_MEM");
        }
        else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
        {
            Serial.println("Peer not found.");
        }
        else
        {
            Serial.println("Unknown error");
        }
        return false;
    }
    */
}
