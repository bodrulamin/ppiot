#include "temperature.h"
#include <Arduino.h>

TemperatureSensor::TemperatureSensor(int pin) : pin(pin) {
    dht = new DHT(pin, DHT22);
    sensorInitialized = false;
    lastTemperature = 0.0;
    lastHumidity = 0.0;
    lastHeatIndex = 0.0;
    lastReadingValid = false;
}

TemperatureSensor::~TemperatureSensor() {
    delete dht;
}

void TemperatureSensor::begin() {
    dht->begin();
    sensorInitialized = true;
    Serial.println("DHT22 Temperature & Humidity Sensor Initialized");
    Serial.print("Sensor pin: GPIO ");
    Serial.println(pin);
    Serial.println("Waiting for first reading...");
}

void TemperatureSensor::readTemperature() {
    // Read temperature and humidity
    float humidity = dht->readHumidity();
    float temperature = dht->readTemperature(); // Celsius by default

    // Check if readings are valid
    if (isnan(humidity) || isnan(temperature)) {
        lastReadingValid = false;
        Serial.println("Error: Failed to read from DHT22 sensor!");
        Serial.println("Possible issues:");
        Serial.print("  - Sensor not connected to GPIO ");
        Serial.println(pin);
        Serial.println("  - Loose wiring or bad sensor");
        Serial.print("  - Check VCC->3.3V (or 5V), GND->GND, DATA->GPIO");
        Serial.println(pin);
        Serial.println("  - Wait a few seconds between readings");
        return;
    }

    // Calculate heat index (feels like temperature)
    float heatIndex = dht->computeHeatIndex(temperature, humidity, false); // false = Celsius

    // Store values
    lastTemperature = temperature;
    lastHumidity = humidity;
    lastHeatIndex = heatIndex;
    lastReadingValid = true;

    // Print readings
    Serial.println("=== DHT22 Readings ===");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Heat Index: ");
    Serial.print(heatIndex);
    Serial.println(" °C");
    Serial.println("====================");
}
