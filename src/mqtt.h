#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "temperature.h"

class MQTTManager {
private:
    WiFiClient espClient;
    PubSubClient* mqttClient;
    const char* mqttServer;
    int mqttPort;
    const char* mqttUser;
    const char* mqttPassword;
    String clientId;
    String baseTopic;

    unsigned long lastReconnectAttempt;
    unsigned long reconnectInterval;
    bool enabled;

    // Sensor references
    TemperatureSensor* tempSensor;
    DS18B20Sensor* ds18b20Sensor;

    // Connection state
    bool wasConnected;

    // Reconnect logic
    bool reconnect();

public:
    MQTTManager(const char* server, int port, const char* user, const char* password);
    ~MQTTManager();

    void begin(TemperatureSensor* tempSensor, DS18B20Sensor* ds18b20Sensor);
    void loop();

    // Publishing methods
    void publishDHT22Data();
    void publishDS18B20Data();
    void publishAllSensorData();

    // Connection status
    bool isConnected();

    // Enable/disable MQTT
    void setEnabled(bool enable);
    bool isEnabled() const { return enabled; }
};

#endif // MQTT_H
