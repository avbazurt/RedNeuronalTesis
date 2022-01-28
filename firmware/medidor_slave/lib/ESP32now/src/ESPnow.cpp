#include "Arduino.h"
#include "ESPnow.h"
#include <WiFi.h>
#include <ArduinoJson.h>

void (*punteroCallback)(char MAC[], char text[]);
bool status_send;

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

    status_send = status == ESP_NOW_SEND_SUCCESS ? true : false;

    log_i("Last Packet Sent to: %s", macStr);
    log_i("Last Packet Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

    // return status == ESP_NOW_SEND_SUCCESS;
}

void ESP32_now::setReciveCallback(void (*puntero)(char MAC[], char text[]))
{
    punteroCallback = puntero;
}

ESP32_now::ESP32_now(String name)
{
    //Guardo el nombre a utilizar
    nameGroup = name;

    WiFi.mode(WIFI_AP_STA);
    String MAC = WiFi.macAddress();
    log_i("My MAC Address is: %", MAC);
    status_send = false;

    //Configuramos el nombre de la red
    MAC.replace(":", "");

    String name_mac = "{NAME}-{MAC}";

    name_mac.replace("{NAME}",name);
    name_mac.replace("{MAC}", MAC);

    WiFi.softAP(name_mac.c_str());

    // shut down wifi
    // WiFi.disconnect();
}

void ESP32_now::begin()
{
    if (esp_now_init() == ESP_OK)
    {
        log_i("ESPNow Init Success");
    }
    else
    {
        log_e("ESPNow Init Failed");
        delay(3000);
        ESP.restart();
    }

    esp_now_register_send_cb(sentCallback);
    esp_now_register_recv_cb(receiveCallback);
}

bool ESP32_now::sentData(uint8_t peerAddress[], const String &message, bool validate_send)
{
    // and this will send a message to a specific device
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, peerAddress, 6);
    
    if (!esp_now_is_peer_exist(peerAddress))
    {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());
    if (validate_send)
    {
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
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

// Global copy of slave
#define NUMSLAVES 20
esp_now_peer_info_t slaves[NUMSLAVES] = {};
int SlaveCnt = 0;

#define CHANNEL 3
#define PRINTSCANRESULTS 0

// Scan for slaves in AP mode
void ESP32_now::ScanForSlave()
{
    int8_t scanResults = WiFi.scanNetworks();
    // reset slaves
    memset(slaves, 0, sizeof(slaves));
    SlaveCnt = 0;
    Serial.println("");
    if (scanResults == 0)
    {
        Serial.println("No WiFi devices in AP Mode found");
    }
    else
    {
        Serial.print("Found ");
        Serial.print(scanResults);
        Serial.println(" devices ");
        for (int i = 0; i < scanResults; ++i)
        {
            // Print SSID and RSSI for each device found
            String SSID = WiFi.SSID(i);
            int32_t RSSI = WiFi.RSSI(i);
            String BSSIDstr = WiFi.BSSIDstr(i);

            if (PRINTSCANRESULTS)
            {
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(SSID);
                Serial.print(" [");
                Serial.print(BSSIDstr);
                Serial.print("]");
                Serial.print(" (");
                Serial.print(RSSI);
                Serial.print(")");
                Serial.println("");
            }
            delay(10);
            // Check if the current device starts with `Slave`
            if (SSID.indexOf("ESP") == 0)
            {
                // SSID of interest
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(SSID);
                Serial.print(" [");
                Serial.print(BSSIDstr);
                Serial.print("]");
                Serial.print(" (");
                Serial.print(RSSI);
                Serial.print(")");
                Serial.println("");
                // Get BSSID => Mac Address of the Slave
                int mac[6];

                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
                {
                    for (int ii = 0; ii < 6; ++ii)
                    {
                        slaves[SlaveCnt].peer_addr[ii] = (uint8_t)mac[ii];
                    }
                }
                slaves[SlaveCnt].channel = CHANNEL; // pick a channel
                slaves[SlaveCnt].encrypt = 0;       // no encryption
                SlaveCnt++;
            }
        }
    }

    if (SlaveCnt > 0)
    {
        Serial.print(SlaveCnt);
        Serial.println(" Slave(s) found, processing..");
    }
    else
    {
        Serial.println("No Slave Found, trying again.");
    }

    // clean up ram
    WiFi.scanDelete();
}
