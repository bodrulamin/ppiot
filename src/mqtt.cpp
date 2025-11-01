#include "mqtt.h"
#include <Arduino.h>
#include "wifi_manager.h"

MQTTManager::MQTTManager(const char* server, int port, const char* user, const char* password)
    : mqttServer(server), mqttPort(port), mqttUser(user), mqttPassword(password) {
    mqttClient = new PubSubClient(espClient);
    lastReconnectAttempt = 0;
    reconnectInterval = 5000; // Try to reconnect every 5 seconds
    enabled = true;
    tempSensor = nullptr;
    ds18b20Sensor = nullptr;
    wasConnected = false;

    // Generate unique client ID using MAC address
    clientId = "ppiot-" + WiFiManager::getMacLastDigits();
    baseTopic = "ppiot/" + WiFiManager::getMacLastDigits();
}

MQTTManager::~MQTTManager() {
    if (mqttClient) {
        mqttClient->disconnect();
        delete mqttClient;
    }
}

void MQTTManager::begin(TemperatureSensor* tempSensor, DS18B20Sensor* ds18b20Sensor) {
    this->tempSensor = tempSensor;
    this->ds18b20Sensor = ds18b20Sensor;

    mqttClient->setServer(mqttServer, mqttPort);

    Serial.println("=== MQTT Manager Initialized ===");
    Serial.print("Server: ");
    Serial.println(mqttServer);
    Serial.print("Port: ");
    Serial.println(mqttPort);
    Serial.print("Client ID: ");
    Serial.println(clientId);
    Serial.print("Base Topic: ");
    Serial.println(baseTopic);
    Serial.println("==============================");
}

bool MQTTManager::reconnect() {
    if (!enabled) {
        return false;
    }

    // Only attempt reconnection if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.print(mqttPort);
    Serial.print(" as ");
    Serial.print(clientId);
    Serial.print("...");

    // Attempt to connect with username and password
    bool connected = false;
    if (mqttUser && strlen(mqttUser) > 0) {
        connected = mqttClient->connect(clientId.c_str(), mqttUser, mqttPassword);
    } else {
        connected = mqttClient->connect(clientId.c_str());
    }

    if (connected) {
        Serial.println(" Connected!");

        // Publish connection status
        String statusTopic = baseTopic + "/status";
        mqttClient->publish(statusTopic.c_str(), "online", true); // retained message

        // Publish device info
        String deviceInfoTopic = baseTopic + "/device/info";
        String deviceInfo = "{\"clientId\":\"" + clientId + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
        mqttClient->publish(deviceInfoTopic.c_str(), deviceInfo.c_str());

        return true;
    } else {
        Serial.print(" Failed, rc=");
        Serial.print(mqttClient->state());
        Serial.println();
        return false;
    }
}

void MQTTManager::loop() {
    if (!enabled) {
        return;
    }

    // Only process MQTT if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        if (wasConnected) {
            Serial.println("[MQTT] WiFi disconnected, MQTT will reconnect when WiFi is back");
            wasConnected = false;
        }
        return;
    }

    if (!mqttClient->connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > reconnectInterval) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                wasConnected = true;
                lastReconnectAttempt = 0;
            }
        }
    } else {
        wasConnected = true;
        mqttClient->loop();
    }
}

void MQTTManager::publishDHT22Data() {
    if (!mqttClient->connected() || !tempSensor || !tempSensor->isValid()) {
        return;
    }

    // Publish individual readings
    String tempTopic = baseTopic + "/dht22/temperature";
    String humidityTopic = baseTopic + "/dht22/humidity";
    String heatIndexTopic = baseTopic + "/dht22/heatindex";

    char tempStr[8];
    char humidityStr[8];
    char heatIndexStr[8];

    dtostrf(tempSensor->getTemperature(), 6, 2, tempStr);
    dtostrf(tempSensor->getHumidity(), 6, 2, humidityStr);
    dtostrf(tempSensor->getHeatIndex(), 6, 2, heatIndexStr);

    mqttClient->publish(tempTopic.c_str(), tempStr);
    mqttClient->publish(humidityTopic.c_str(), humidityStr);
    mqttClient->publish(heatIndexTopic.c_str(), heatIndexStr);

    // Publish combined JSON data
    String dataTopic = baseTopic + "/dht22/data";
    String jsonData = "{\"temperature\":" + String(tempStr) +
                      ",\"humidity\":" + String(humidityStr) +
                      ",\"heatIndex\":" + String(heatIndexStr) + "}";

    mqttClient->publish(dataTopic.c_str(), jsonData.c_str());

    Serial.print("[MQTT] Published DHT22 data to ");
    Serial.println(dataTopic);
}

void MQTTManager::publishDS18B20Data() {
    if (!mqttClient->connected() || !ds18b20Sensor || !ds18b20Sensor->isValid()) {
        return;
    }

    // Publish DS18B20 temperature
    String tempTopic = baseTopic + "/ds18b20/temperature";

    char tempStr[8];
    dtostrf(ds18b20Sensor->getTemperature(), 6, 2, tempStr);

    mqttClient->publish(tempTopic.c_str(), tempStr);

    // Publish JSON data
    String dataTopic = baseTopic + "/ds18b20/data";
    String jsonData = "{\"temperature\":" + String(tempStr) + "}";

    mqttClient->publish(dataTopic.c_str(), jsonData.c_str());

    Serial.print("[MQTT] Published DS18B20 data to ");
    Serial.println(dataTopic);
}

void MQTTManager::publishAllSensorData() {
    publishDHT22Data();
    publishDS18B20Data();
}

bool MQTTManager::isConnected() {
    return mqttClient->connected();
}

void MQTTManager::setEnabled(bool enable) {
    enabled = enable;
    if (!enabled && mqttClient->connected()) {
        // Publish offline status before disconnecting
        String statusTopic = baseTopic + "/status";
        mqttClient->publish(statusTopic.c_str(), "offline", true);
        mqttClient->disconnect();
    }
}
