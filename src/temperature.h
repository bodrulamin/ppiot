#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
private:
    DHT* dht;
    int pin;
    bool sensorInitialized;
    float lastTemperature;
    float lastHumidity;
    float lastHeatIndex;
    bool lastReadingValid;

public:
    TemperatureSensor(int pin);
    ~TemperatureSensor();

    void begin();
    void readTemperature();

    // Getter methods for current readings
    float getTemperature() const { return lastTemperature; }
    float getHumidity() const { return lastHumidity; }
    float getHeatIndex() const { return lastHeatIndex; }
    bool isValid() const { return lastReadingValid; }
};

class DS18B20Sensor {
private:
    OneWire* oneWire;
    DallasTemperature* sensors;
    int pin;
    bool sensorInitialized;
    float lastTemperature;
    bool lastReadingValid;
    int deviceCount;

public:
    DS18B20Sensor(int pin);
    ~DS18B20Sensor();

    void begin();
    void readTemperature();

    // Getter methods for current readings
    float getTemperature() const { return lastTemperature; }
    bool isValid() const { return lastReadingValid; }
    int getDeviceCount() const { return deviceCount; }
};

#endif // TEMPERATURE_H
