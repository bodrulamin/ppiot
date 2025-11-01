#include <Arduino.h>
#include <WiFi.h>
#include <esp_chip_info.h>
#include <esp_system.h>
#include "webserver.h"
#include "webserver_html.h"
#include "dashboard_html.h"
#include "deviceinfo_html.h"
#include "config.h"

WebServer::WebServer(WiFiManager* wifiMgr, TemperatureSensor* tempSens, DS18B20Sensor* ds18b20Sens, bool* apMode) {
    server = new AsyncWebServer(80);
    wifiManager = wifiMgr;
    tempSensor = tempSens;
    ds18b20Sensor = ds18b20Sens;
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

    // Device info route - serve device information page
    server->on("/device", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", deviceinfo_html);
    });

    // API endpoint for sensor data
    server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"dht22\":{";
        json += "\"temperature\":" + String(tempSensor->getTemperature(), 2) + ",";
        json += "\"humidity\":" + String(tempSensor->getHumidity(), 2) + ",";
        json += "\"heatIndex\":" + String(tempSensor->getHeatIndex(), 2) + ",";
        json += "\"valid\":" + String(tempSensor->isValid() ? "true" : "false");
        json += "},";
        json += "\"ds18b20\":{";
        json += "\"temperature\":" + String(ds18b20Sensor->getTemperature(), 2) + ",";
        json += "\"valid\":" + String(ds18b20Sensor->isValid() ? "true" : "false");
        json += "}";
        json += "}";
        request->send(200, "application/json", json);
    });

    // API endpoint for device information
    server->on("/api/device", HTTP_GET, [](AsyncWebServerRequest *request){
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        // System info
        String chipModel = "ESP32";
        uint8_t cpuCores = chip_info.cores;
        uint32_t cpuFreq = ESP.getCpuFreqMHz();
        String sdkVersion = String(ESP.getSdkVersion());

        // RAM info
        uint32_t totalRam = ESP.getHeapSize();
        uint32_t freeRam = ESP.getFreeHeap();
        uint32_t usedRam = totalRam - freeRam;
        uint32_t largestBlock = ESP.getMaxAllocHeap();

        // Flash info
        uint32_t flashSize = ESP.getFlashChipSize();
        uint32_t flashSpeed = ESP.getFlashChipSpeed();
        uint32_t sketchSize = ESP.getSketchSize();
        String flashMode;
        switch(ESP.getFlashChipMode()) {
            case FM_QIO:  flashMode = "QIO"; break;
            case FM_QOUT: flashMode = "QOUT"; break;
            case FM_DIO:  flashMode = "DIO"; break;
            case FM_DOUT: flashMode = "DOUT"; break;
            default:      flashMode = "UNKNOWN"; break;
        }

        // WiFi info
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        String wifiSSID = WiFi.SSID();
        String ipAddress = WiFi.localIP().toString();
        String macAddress = WiFi.macAddress();
        int32_t rssi = WiFi.RSSI();
        String gateway = WiFi.gatewayIP().toString();

        // Runtime info
        uint32_t uptime = millis();
        String resetReason;
        switch(esp_reset_reason()) {
            case ESP_RST_POWERON:   resetReason = "Power On"; break;
            case ESP_RST_SW:        resetReason = "Software Reset"; break;
            case ESP_RST_PANIC:     resetReason = "Panic/Exception"; break;
            case ESP_RST_INT_WDT:   resetReason = "Interrupt Watchdog"; break;
            case ESP_RST_TASK_WDT:  resetReason = "Task Watchdog"; break;
            case ESP_RST_WDT:       resetReason = "Watchdog"; break;
            case ESP_RST_DEEPSLEEP: resetReason = "Deep Sleep"; break;
            case ESP_RST_BROWNOUT:  resetReason = "Brownout"; break;
            default:                resetReason = "Unknown"; break;
        }

        String json = "{";
        json += "\"chip_model\":\"" + chipModel + "\",";
        json += "\"cpu_cores\":" + String(cpuCores) + ",";
        json += "\"cpu_freq\":" + String(cpuFreq) + ",";
        json += "\"sdk_version\":\"" + sdkVersion + "\",";

        json += "\"total_ram\":" + String(totalRam) + ",";
        json += "\"free_ram\":" + String(freeRam) + ",";
        json += "\"used_ram\":" + String(usedRam) + ",";
        json += "\"largest_block\":" + String(largestBlock) + ",";

        json += "\"flash_size\":" + String(flashSize) + ",";
        json += "\"flash_speed\":" + String(flashSpeed) + ",";
        json += "\"flash_mode\":\"" + flashMode + "\",";
        json += "\"sketch_size\":" + String(sketchSize) + ",";

        json += "\"wifi_connected\":" + String(wifiConnected ? "true" : "false") + ",";
        json += "\"wifi_ssid\":\"" + wifiSSID + "\",";
        json += "\"ip_address\":\"" + ipAddress + "\",";
        json += "\"mac_address\":\"" + macAddress + "\",";
        json += "\"rssi\":" + String(rssi) + ",";
        json += "\"gateway\":\"" + gateway + "\",";

        json += "\"uptime\":" + String(uptime) + ",";
        json += "\"reset_reason\":\"" + resetReason + "\"";
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

    // Info route - return saved credentials info and connection status
    server->on("/info", HTTP_GET, [this](AsyncWebServerRequest *request){
        String savedSSID = "";
        String savedPassword = "";
        bool hasCredentials = wifiManager->loadSavedCredentials(savedSSID, savedPassword);
        bool isConnected = (WiFi.status() == WL_CONNECTED);
        String currentSSID = WiFi.SSID();
        String ipAddress = WiFi.localIP().toString();

        String json = "{";
        json += "\"saved_ssid\":\"" + savedSSID + "\",";
        json += "\"has_saved\":" + String(hasCredentials ? "true" : "false") + ",";
        json += "\"connected\":" + String(isConnected ? "true" : "false") + ",";
        json += "\"current_ssid\":\"" + currentSSID + "\",";
        json += "\"ip_address\":\"" + ipAddress + "\"";
        json += "}";

        request->send(200, "application/json", json);
    });

    // Disconnect from WiFi
    server->on("/disconnect", HTTP_GET, [this](AsyncWebServerRequest *request){
        Serial.println("[DISCONNECT] Disconnecting from WiFi...");
        WiFi.disconnect();
        *isAPMode = true;
        request->send(200, "text/plain", "Disconnected from WiFi");
    });

    // Clear saved credentials
    server->on("/clear", HTTP_GET, [this](AsyncWebServerRequest *request){
        Serial.println("[CLEAR] Clearing saved WiFi credentials...");
        wifiManager->clearCredentials();
        WiFi.disconnect();
        *isAPMode = true;
        request->send(200, "text/plain", "Credentials cleared");
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
        delay(1000);
        WiFi.disconnect();
        delay(1000);

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
