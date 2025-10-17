#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "temperature.h"
#include "wifi_manager.h"
#include "webserver.h"

// Global objects
TemperatureSensor* tempSensor = nullptr;
WiFiManager* wifiManager = nullptr;
WebServer* webServer = nullptr;

// State variables
bool isAPMode = false;
unsigned long buttonPressStart = 0;
bool buttonPressed = false;

// LED blink variables
unsigned long lastLEDToggle = 0;
bool ledState = false;

// Temperature reading variables
unsigned long lastTempRead = 0;

// Function to handle factory reset
void checkFactoryReset() {
    Serial.println("Checking for factory reset button press...");
    unsigned long resetCheckStart = millis();
    bool factoryReset = false;

    // Check if button is held for RESET_HOLD_TIME at startup
    while (millis() - resetCheckStart < RESET_HOLD_TIME) {
        if (digitalRead(RESET_BUTTON_PIN) == HIGH) {
            // Button released, stop checking
            break;
        }

        // Blink LED fast to indicate reset mode
        digitalWrite(LED_PIN, (millis() / 200) % 2);

        if (millis() - resetCheckStart >= RESET_HOLD_TIME) {
            factoryReset = true;
            Serial.println("\n[RESET] Factory reset triggered!");

            // Clear all saved credentials
            wifiManager->clearCredentials();

            Serial.println("[RESET] WiFi credentials cleared!");

            // Fast blink to confirm reset
            for (int i = 0; i < 10; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
                delay(100);
            }
        }
    }

    digitalWrite(LED_PIN, LOW);

    if (factoryReset) {
        Serial.println("[RESET] Starting in AP mode after factory reset");
        isAPMode = true;
    }
}

// Function to start Access Point Mode
void startAPMode() {
    isAPMode = true;
    String apSSID = "ppiot-" + WiFiManager::getMacLastDigits();

    Serial.println("Starting Access Point Mode");
    Serial.print("AP SSID: ");
    Serial.println(apSSID);
    Serial.print("AP Password: ");
    Serial.println(AP_PASSWORD);

    WiFi.mode(WIFI_AP_STA); // Always use AP+STA mode
    WiFi.softAP(apSSID.c_str(), AP_PASSWORD); // Always visible

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Start web server
    webServer->begin();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // Use internal pullup resistor

    delay(1000);
    Serial.println("\n\n=== PPIOT Device Starting ===");

    // Initialize temperature sensor
    tempSensor = new TemperatureSensor(TEMP_SENSOR_PIN);
    tempSensor->begin();

    // Initialize WiFi manager
    wifiManager = new WiFiManager();
    wifiManager->begin();

    // Initialize web server (pass wifiManager, tempSensor, and isAPMode flag)
    webServer = new WebServer(wifiManager, tempSensor, &isAPMode);

    // Check for factory reset button press
    checkFactoryReset();

    if (isAPMode) {
        // Already in AP mode from factory reset
        startAPMode();
        return;
    }

    // Try to connect with saved credentials
    if (wifiManager->hasCredentials()) {
        Serial.println("Found saved WiFi credentials");

        // Always start AP mode first
        startAPMode();

        if (wifiManager->connectToWiFi()) {
            isAPMode = false;
            Serial.println("Device connected to WiFi - AP running in background");
        } else {
            Serial.println("Failed to connect to saved WiFi");
            Serial.println("AP visible for configuration, continuously trying to reconnect...");
            isAPMode = true;
        }
    } else {
        Serial.println("No saved credentials found");
        startAPMode();
    }
}

void loop() {
    // Handle async web server save request
    if (webServer->isSaveInProgress()) {
        webServer->handleSaveRequest();
        return;
    }

    // Handle async web server check request
    if (webServer->isCheckInProgress()) {
        webServer->handleCheckRequest();
        return;
    }

    // Handle async web server retry request
    if (webServer->isRetryInProgress()) {
        webServer->handleRetryRequest();
        return;
    }

    // Auto-reconnect logic
    if (!webServer->isRetryInProgress()) {
        wifiManager->handleAutoReconnect();

        // Update isAPMode flag based on reconnection status
        if (wifiManager->isAutoReconnecting() && WiFi.status() == WL_CONNECTED && isAPMode) {
            Serial.println("[AUTO-RECONNECT] Connected! AP still running in background");
            isAPMode = false;
        }
    }

    // Non-blocking LED blink - only when in AP mode (hotspot active)
    if (isAPMode) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastLEDToggle >= LED_BLINK_INTERVAL) {
            lastLEDToggle = currentMillis;
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        }
    } else {
        // Keep LED off when connected to WiFi
        digitalWrite(LED_PIN, LOW);
    }

    // Read temperature periodically
    unsigned long currentMillis = millis();
    if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
        lastTempRead = currentMillis;
        tempSensor->readTemperature();  // Comment this line to disable temperature reading
    }
}
