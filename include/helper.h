#ifndef BATTERY_SERVICE_H
#define BATTERY_SERVICE_H

#include <Arduino.h>
#include <constants.h>


// Include the pin definitions
class Helper
{

public:
    Helper() {} // Initialize in the constructor

    int batteryPercentage()
    {
        int batteryVoltage = (analogRead(BATTERY) * 3.6 * 5) / 4096.0;  // Convert analog reading to voltage
        int batteryPercentage = map(batteryVoltage, 9.4, 14.2, 0, 100); // Map voltage range to percentage

        // Clamp the percentage value between 0 and 100
        batteryPercentage = constrain(batteryPercentage, 0, 100);

        return batteryPercentage;
    }


    void restartDevice(int pin)
    {
        digitalWrite(pin, LOW);
        delay(1000 * 30);
        digitalWrite(pin, HIGH);
    }
};

#endif