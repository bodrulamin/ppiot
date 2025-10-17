#ifndef PPIOT_WEBSERVER_H
#define PPIOT_WEBSERVER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "wifi_manager.h"
#include "temperature.h"

class WebServer {
private:
    AsyncWebServer* server;
    WiFiManager* wifiManager;
    TemperatureSensor* tempSensor;
    bool* isAPMode;

    // Operation state variables
    bool scanInProgress;
    bool retryInProgress;
    unsigned long retryStartTime;
    AsyncWebServerRequest* retryRequest;

    bool saveInProgress;
    unsigned long saveStartTime;
    AsyncWebServerRequest* saveRequest;
    String newSSID;
    String newPassword;

    bool checkInProgress;
    unsigned long checkStartTime;
    AsyncWebServerRequest* checkRequest;
    String checkSSID;
    String checkPassword;

    void setupRoutes();

public:
    WebServer(WiFiManager* wifiMgr, TemperatureSensor* tempSens, bool* apMode);
    ~WebServer();

    void begin();
    void handleSaveRequest();
    void handleCheckRequest();
    void handleRetryRequest();

    bool isSaveInProgress() const { return saveInProgress; }
    bool isCheckInProgress() const { return checkInProgress; }
    bool isRetryInProgress() const { return retryInProgress; }
};

#endif // PPIOT_WEBSERVER_H
