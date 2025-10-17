#include <Arduino.h>
#include <WiFi.h>
#include "webserver.h"
#include "webserver_html.h"
#include "dashboard_html.h"
#include "config.h"

WebServer::WebServer(WiFiManager* wifiMgr, TemperatureSensor* tempSens, bool* apMode) {
    server = new AsyncWebServer(80);
    wifiManager = wifiMgr;
    tempSensor = tempSens;
    isAPMode = apMode;

    scanInProgress = false;
    retryInProgress = false;
    retryStartTime = 0;
    retryRequest = nullptr;

    saveInProgress = false;
    saveStartTime = 0;
    saveRequest = nullptr;
    newSSID = "";
    newPassword = "";

    checkInProgress = false;
    checkStartTime = 0;
    checkRequest = nullptr;
    checkSSID = "";
    checkPassword = "";
}

WebServer::~WebServer() {
    delete server;
}

void WebServer::begin() {
    setupRoutes();
    server->begin();
    Serial.println("Web server started");
}

void WebServer::setupRoutes() {
    // Root route - serve WiFi configuration page
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    // Dashboard route - serve temperature/humidity page
    server->on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", dashboard_html);
    });

    // API endpoint for sensor data
    server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"temperature\":" + String(tempSensor->getTemperature(), 2) + ",";
        json += "\"humidity\":" + String(tempSensor->getHumidity(), 2) + ",";
        json += "\"heatIndex\":" + String(tempSensor->getHeatIndex(), 2) + ",";
        json += "\"valid\":" + String(tempSensor->isValid() ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json);
    });

    // WiFi scan route
    server->on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "[";
        int n = WiFi.scanComplete();

        if (n == -2) {
            // Scan not triggered yet, start it
            if (!scanInProgress) {
                Serial.println("[SCAN] Starting WiFi network scan...");
                scanInProgress = true;
                WiFi.scanNetworks(true);
            }
            json += "]";
            request->send(200, "application/json", json);
        } else if (n == -1) {
            // Scan in progress - don't log repeatedly
            json += "]";
            request->send(200, "application/json", json);
        } else {
            // Scan complete, return results
            if (scanInProgress) {
                Serial.print("[SCAN] Scan complete! Found ");
                Serial.print(n);
                Serial.println(" networks:");

                for (int i = 0; i < n; ++i) {
                    Serial.print("  ");
                    Serial.print(i + 1);
                    Serial.print(". ");
                    Serial.print(WiFi.SSID(i));
                    Serial.print(" (");
                    Serial.print(WiFi.RSSI(i));
                    Serial.print(" dBm) ");
                    Serial.println(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "[Secured]" : "[Open]");
                }
                scanInProgress = false;
            }

            for (int i = 0; i < n; ++i) {
                if (i) json += ",";
                json += "{";
                json += "\"rssi\":" + String(WiFi.RSSI(i));
                json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
                json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
                json += ",\"channel\":" + String(WiFi.channel(i));
                json += ",\"secure\":" + String(WiFi.encryptionType(i));
                json += "}";
            }

            WiFi.scanDelete();
            // Prepare for next scan
            if (WiFi.scanComplete() == -2) {
                WiFi.scanNetworks(true);
                scanInProgress = true;
            }
            json += "]";
            request->send(200, "application/json", json);
        }
    });

    // Retry connection route
    server->on("/retry", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (retryInProgress) {
            request->send(400, "text/plain", "Retry already in progress");
            return;
        }

        // Get saved credentials
        String savedSSID = "";
        String savedPassword = "";
        if (!wifiManager->loadSavedCredentials(savedSSID, savedPassword)) {
            request->send(400, "text/plain", "No saved credentials found");
            return;
        }

        Serial.println("[RETRY] Attempting to reconnect to saved WiFi...");
        Serial.print("[RETRY] SSID: ");
        Serial.println(savedSSID);

        // Start connection attempt
        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

        retryInProgress = true;
        retryStartTime = millis();
        retryRequest = request;
    });

    // Info route - return saved credentials info
    server->on("/info", HTTP_GET, [this](AsyncWebServerRequest *request){
        String savedSSID = "";
        String savedPassword = "";
        bool hasCredentials = wifiManager->loadSavedCredentials(savedSSID, savedPassword);

        String json = "{";
        json += "\"saved_ssid\":\"" + savedSSID + "\",";
        json += "\"has_saved\":";
        json += hasCredentials ? "true" : "false";
        json += "}";

        request->send(200, "application/json", json);
    });

    // Check connection route
    server->on("/check", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (checkInProgress) {
            request->send(400, "text/plain", "Check already in progress");
            return;
        }

        if (saveInProgress || retryInProgress) {
            request->send(400, "text/plain", "Another operation in progress");
            return;
        }

        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            checkSSID = request->getParam("ssid", true)->value();
            checkPassword = request->getParam("password", true)->value();

            Serial.println("[CHECK] Testing WiFi credentials:");
            Serial.print("[CHECK] SSID: ");
            Serial.println(checkSSID);

            // Start connection test
            WiFi.begin(checkSSID.c_str(), checkPassword.c_str());

            checkInProgress = true;
            checkStartTime = millis();
            checkRequest = request;
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    // Save credentials route
    server->on("/save", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (saveInProgress) {
            request->send(400, "text/plain", "Save already in progress");
            return;
        }

        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            newSSID = request->getParam("ssid", true)->value();
            newPassword = request->getParam("password", true)->value();

            Serial.println("Received WiFi credentials:");
            Serial.print("SSID: ");
            Serial.println(newSSID);

            // Try to connect to the new WiFi
            Serial.println("Testing connection to new WiFi...");

            WiFi.begin(newSSID.c_str(), newPassword.c_str());

            saveInProgress = true;
            saveStartTime = millis();
            saveRequest = request;
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });
}

void WebServer::handleSaveRequest() {
    if (!saveInProgress || saveRequest == nullptr) {
        return;
    }

    unsigned long elapsed = millis() - saveStartTime;

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nSuccessfully connected to new WiFi!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // Save credentials
        wifiManager->saveCredentials(newSSID, newPassword);

        // Send success response
        saveRequest->send(200, "text/plain", "Connected successfully! Credentials saved.");
        saveRequest = nullptr;
        saveInProgress = false;

        Serial.println("New credentials saved and connected successfully!");
        Serial.println("Device is now connected to WiFi. AP still running in background.");

        // Switch to connected mode
        *isAPMode = false;
    } else if (elapsed > 10000) {
        // Timeout after 10 seconds
        Serial.println("\nFailed to connect to new WiFi. Keeping old credentials.");
        WiFi.disconnect();

        // Send failure response
        saveRequest->send(400, "text/plain", "Failed to connect. Please check credentials.");
        saveRequest = nullptr;
        saveInProgress = false;
        Serial.println("Connection failed - please try again");
    }
}

void WebServer::handleCheckRequest() {
    if (!checkInProgress) {
        return;
    }

    unsigned long elapsed = millis() - checkStartTime;

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[CHECK] Connection test successful!");
        Serial.print("[CHECK] IP address: ");
        Serial.println(WiFi.localIP());

        // Send response if request is still valid
        if (checkRequest != nullptr) {
            checkRequest->send(200, "text/plain", "Connection successful! Credentials are valid.");
            checkRequest = nullptr;
        }

        checkInProgress = false;

        // Disconnect from test connection after a delay to ensure response is sent
        delay(200);
        WiFi.disconnect();
        delay(100);

        Serial.println("[CHECK] Disconnected from test WiFi");
    } else if (elapsed > 10000) {
        // Timeout after 10 seconds
        Serial.println("\n[CHECK] Connection test failed - timeout");

        // Send response if request is still valid
        if (checkRequest != nullptr) {
            checkRequest->send(400, "text/plain", "Connection failed. Please check credentials.");
            checkRequest = nullptr;
        }

        checkInProgress = false;

        // Disconnect from failed attempt
        WiFi.disconnect();
    }
}

void WebServer::handleRetryRequest() {
    if (!retryInProgress || retryRequest == nullptr) {
        return;
    }

    unsigned long elapsed = millis() - retryStartTime;

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[RETRY] Successfully reconnected!");
        Serial.print("[RETRY] IP address: ");
        Serial.println(WiFi.localIP());

        // Send success response
        retryRequest->send(200, "text/plain", "Connected successfully!");

        retryRequest = nullptr;
        retryInProgress = false;

        Serial.println("[RETRY] Device is now connected to WiFi. AP still running in background.");

        // Switch to connected mode
        *isAPMode = false;
    } else if (elapsed > 10000) {
        // Timeout after 10 seconds
        Serial.println("\n[RETRY] Failed to reconnect - timeout");
        WiFi.disconnect();

        retryRequest->send(400, "text/plain", "Failed to connect. WiFi may be down or password changed.");
        retryRequest = nullptr;
        retryInProgress = false;
    }
}
