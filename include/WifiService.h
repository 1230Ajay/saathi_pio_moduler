#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include <WiFi.h>
#include "constants.h"

class WiFiService {

public:
    void begin() {
        // Set WiFi to station mode
        WiFi.mode(WIFI_STA);
        
        // Connect to the Wi-Fi network
        Serial.println("Connecting to WiFi...");
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        delay(1000*10);
    }

    bool isConnected() {
        // Check if the device is connected to WiFi
        return WiFi.status() == WL_CONNECTED;
    }

    void disconnect() {
        // Disconnect from Wi-Fi
        WiFi.disconnect();
        Serial.println("Disconnected from WiFi.");
    }
};

#endif // WIFISERVICE_H
