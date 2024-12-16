#include "SocketIOclient.h"
#include "ArduinoJson.h"
#include "helper.h"

class SocketService
{
private:
  
    Helper helper;

public:
    SocketIOclient socketIO;
    // Constructor
    SocketService() {
      
    }

    // Initialize the socket connection
    void connect(const char *host, int port)
    {
        Serial.println("Connecting to socket...");
        socketIO.beginSSL(host, port, "/socket.io/?EIO=4");
        socketIO.onEvent([this](socketIOmessageType_t type, uint8_t *payload, size_t length)
                         { this->socketIOEvent(type, payload, length); });


    }

    // Handle socket events
    void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
    {
        switch (type)
        {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        

        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");
            sendDeviceConnectedEvent();
            break;
            
        case sIOtype_EVENT:
            if (strncmp((char *)payload, "[\"pong\"]", 8) == 0) {
                Serial.println("Pong received, connection is alive");
            }
            handleSocketEvent(payload, length);
            break;

        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            break;
        }
    }

    void send_log(int battery_percentage, bool sensor_status,bool isTrainDetected=false)
    {
        DynamicJsonDocument docOut(1024);
        JsonArray array = docOut.to<JsonArray>();
        array.add("updateDevice");
        JsonObject eventData = docOut.createNestedObject();
        eventData["battery"] = battery_percentage;
        eventData["sensor_status"] = sensor_status;
        eventData["isTrainDetected"] = isTrainDetected;
        String output;
        serializeJson(docOut, output);
        socketIO.sendEVENT(output.c_str());

        Serial.print("log sent : ");
        Serial.println(output);
    }



     // Send device connected event
    void sendDeviceConnectedEvent()
    {
        Serial.println("Sending device connected event...");
        DynamicJsonDocument docOut(1024);
        JsonArray array = docOut.to<JsonArray>();
        array.add("deviceConnect");
        array.add(UID); // Send the UID as the user ID
        String output;
        serializeJson(docOut, output);
        socketIO.sendEVENT(output.c_str());
        Serial.println("Device connected event sent");
    }
    // Send log event



    // Check if the socket is connected
    bool isConnected()
    {
        return socketIO.isConnected();
    }

    // Handle incoming socket events
    void handleSocketEvent(uint8_t *payload, size_t length)
    {
        DynamicJsonDocument docIn(1024);
        DeserializationError error = deserializeJson(docIn, payload, length);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        // Extract the event name
        const char *eventName = docIn[0];

        if (strcmp(eventName, "setData") == 0)
        {
            JsonObject data = docIn[1].as<JsonObject>();
            Serial.println(data);
        }
    }
};
