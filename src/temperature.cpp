#include "temperature.h"
#include <Arduino.h>

TemperatureSensor::TemperatureSensor(int pin) {
    oneWire = new OneWire(pin);
    tempSensor = new DallasTemperature(oneWire);
    sensorInitialized = false;
}

TemperatureSensor::~TemperatureSensor() {
    delete tempSensor;
    delete oneWire;
}

void TemperatureSensor::begin() {
    tempSensor->begin();
    Serial.println("DS18B20 Temperature Sensor Initialized");

    // Check if sensor is connected
    int deviceCount = tempSensor->getDeviceCount();
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" DS18B20 sensor(s)");

    if (deviceCount == 0) {
        Serial.println("WARNING: No DS18B20 sensors found!");
        Serial.println("Check wiring: VCC->3.3V, GND->GND, DATA->D4, and 4.7K pull-up resistor");
    }
}

void TemperatureSensor::readTemperature() {
    // Re-initialize sensor if needed (some sensors need this)
    if (!sensorInitialized) {
        tempSensor->begin();
        sensorInitialized = true;
        Serial.println("Temperature sensor re-initialized in readTemperature()");
    }

    tempSensor->requestTemperatures(); // Request temperature reading
    float temperature = tempSensor->getTempCByIndex(0); // Get temperature in Celsius

    // Debug: Print raw temperature value
    Serial.print("Raw temperature value: ");
    Serial.println(temperature);

    // Check if reading is valid
    if (temperature != DEVICE_DISCONNECTED_C && temperature != -127.0) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");
    } else {
        Serial.println("Error: Could not read temperature sensor!");
        Serial.println("Possible issues:");
        Serial.println("  - Sensor not connected to D4 (GPIO 4)");
        Serial.println("  - Missing 4.7K pull-up resistor between VCC and DATA");
        Serial.println("  - Loose wiring or bad sensor");
        Serial.println("  - Wrong pin number");

        // Try to re-detect devices
        int count = tempSensor->getDeviceCount();
        Serial.print("Devices detected: ");
        Serial.println(count);
    }
}

int TemperatureSensor::getDeviceCount() {
    return tempSensor->getDeviceCount();
}
