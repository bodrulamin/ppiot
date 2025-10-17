#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
private:
    OneWire* oneWire;
    DallasTemperature* tempSensor;
    bool sensorInitialized;

public:
    TemperatureSensor(int pin);
    ~TemperatureSensor();

    void begin();
    void readTemperature();
    int getDeviceCount();
};

#endif // TEMPERATURE_H
