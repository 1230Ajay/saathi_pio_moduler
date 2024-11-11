#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include <WiFiMulti.h>
#include "constants.h"

class WiFiService {

WiFiMulti wifiMulti;

public:
    void begin() {
        // Set WiFi to station mode
        wifiMulti.addAP(SECRET_SSID,SECRET_PASS);


    }

    bool isConnected() {
        return wifiMulti.run() == WL_CONNECTED;
    }
};

#endif // WIFISERVICE_H
