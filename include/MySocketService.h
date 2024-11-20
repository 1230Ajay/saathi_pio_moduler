#include "SocketIOclient.h"
#include "ArduinoJson.h"
#include "helper.h"

class SocketService
{
private:
    SocketIOclient socketIO;
    Helper helper;

public:
    // Constructor
    SocketService() {}

    // Initialize the socket connection
    void connect(const char *host, int port)
    {
        Serial.println("Connecting to socket...");
        socketIO.begin(host, port, "/socket.io/?EIO=4");
        socketIO.onEvent([this](socketIOmessageType_t type, uint8_t *payload, size_t length)
                         { this->socketIOEvent(type, payload, length); });

        unsigned long startAttemptTime = millis();

        while (!socketIO.isConnected() && millis() - startAttemptTime < 10000)
        { // 10 seconds timeout
            delay(1000*10);
            Serial.print(".");
        }

        if (socketIO.isConnected())
        {
            sendDeviceConnectedEvent();
            Serial.println("\nSocket connected");
        }
        else
        {
            Serial.println("\nFailed to connect to socket");
        }
    }

    // Handle socket events
    void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
    {
        switch (type)
        {
        case sIOtype_DISCONNECT:

            break;
        case sIOtype_CONNECT:
            Serial.println("Socket connected");
            socketIO.send(sIOtype_CONNECT, "/");
            sendDeviceConnectedEvent();
            break;
        case sIOtype_EVENT:
    
            handleSocketEvent(payload, length);
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

    // Loop function for maintaining the connection
    void loop()
    {
        socketIO.loop();
    }

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
