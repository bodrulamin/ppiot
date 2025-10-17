#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <DHT.h>

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

#endif // TEMPERATURE_H
