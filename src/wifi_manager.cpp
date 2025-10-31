#include "wifi_manager.h"
#include "config.h"
#include <Arduino.h>

WiFiManager::WiFiManager() {
    ssid = "";
    password = "";
    hasStoredCredentials = false;
    lastWiFiCheck = 0;
    autoReconnecting = false;
    autoReconnectStartTime = 0;
}

void WiFiManager::begin() {
    // Try to load saved credentials from NVS
    if (preferences.begin("wifi", true)) {
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
        preferences.end();
        hasStoredCredentials = (ssid.length() > 0);
    } else {
        ssid = "";
        password = "";
        hasStoredCredentials = false;
    }

    if (!hasStoredCredentials) {
        Serial.println("No saved credentials (first boot or after factory reset)");
    } else {
        Serial.println("Found saved WiFi credentials");
    }
}

bool WiFiManager::loadSavedCredentials(String &savedSSID, String &savedPassword) {
    // Use cached flag to avoid repeated NVS reads when no credentials exist
    if (!hasStoredCredentials) {
        savedSSID = "";
        savedPassword = "";
        return false;
    }

    if (preferences.begin("wifi", true)) {
        savedSSID = preferences.getString("ssid", "");
        savedPassword = preferences.getString("password", "");
        preferences.end();
        return savedSSID.length() > 0;
    }
    savedSSID = "";
    savedPassword = "";
    return false;
}

bool WiFiManager::saveCredentials(const String &newSSID, const String &newPassword) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPassword);
    preferences.end();

    // Update cache flag and current credentials
    hasStoredCredentials = true;
    ssid = newSSID;
    password = newPassword;

    Serial.println("Credentials saved to NVS");
    return true;
}

void WiFiManager::clearCredentials() {
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();

    // Clear cache flag
    hasStoredCredentials = false;
    ssid = "";
    password = "";

    Serial.println("WiFi credentials cleared!");
}

bool WiFiManager::connectToWiFi() {
    if (ssid.length() == 0) {
        return false;
    }

    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(ssid);

    // If currently in AP mode only, switch to AP+STA mode to allow connection
    wifi_mode_t currentMode = WiFi.getMode();
    if (currentMode == WIFI_AP) {
        Serial.println("Switching to AP+STA mode for connection attempt...");
        WiFi.mode(WIFI_AP_STA);
        delay(100); // Give some time for mode switch
    }

    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        // If connection failed and we were in AP mode, switch back to AP only
        if (currentMode == WIFI_AP) {
            WiFi.mode(WIFI_AP);
        }
        return false;
    }
}

void WiFiManager::handleAutoReconnect() {
    unsigned long currentMillis = millis();

    // Check WiFi status periodically
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = currentMillis;

        wl_status_t status = WiFi.status();

        // Check if WiFi is disconnected
        if (status != WL_CONNECTED) {
            if (!autoReconnecting) {
                // Check if we have credentials before attempting reconnection
                if (!hasStoredCredentials) {
                    // No credentials saved, skip auto-reconnect entirely
                    return;
                }

                // Start new reconnection attempt
                Serial.println("\n[AUTO-RECONNECT] WiFi disconnected! Attempting to reconnect...");
                Serial.print("[AUTO-RECONNECT] Status: ");

                // Log the specific WiFi status
                switch(status) {
                    case WL_NO_SSID_AVAIL:
                        Serial.println("SSID not available");
                        break;
                    case WL_CONNECT_FAILED:
                        Serial.println("Connection failed");
                        break;
                    case WL_CONNECTION_LOST:
                        Serial.println("Connection lost");
                        break;
                    case WL_DISCONNECTED:
                        Serial.println("Disconnected");
                        break;
                    case WL_IDLE_STATUS:
                        Serial.println("Idle/blocked");
                        break;
                    default:
                        Serial.println(status);
                        break;
                }

                // Get saved credentials
                String savedSSID = "";
                String savedPassword = "";

                if (loadSavedCredentials(savedSSID, savedPassword)) {
                    Serial.print("[AUTO-RECONNECT] Connecting to: ");
                    Serial.println(savedSSID);

                    // If in AP mode only, switch to AP+STA mode for reconnection
                    wifi_mode_t currentMode = WiFi.getMode();
                    if (currentMode == WIFI_AP) {
                        Serial.println("[AUTO-RECONNECT] Switching to AP+STA mode...");
                        WiFi.mode(WIFI_AP_STA);
                        delay(100);
                    }

                    // Disconnect first to clear any stuck state
                    WiFi.disconnect(true);
                    delay(100);

                    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
                    autoReconnecting = true;
                    autoReconnectStartTime = currentMillis;
                }
            } else {
                // Already trying to reconnect, check progress
                unsigned long reconnectElapsed = currentMillis - autoReconnectStartTime;

                if (reconnectElapsed > AUTO_RECONNECT_TIMEOUT) {
                    // Reconnection timeout - restart the attempt
                    Serial.println("\n[AUTO-RECONNECT] Timeout! Retrying...");

                    // Get saved credentials again
                    String savedSSID = "";
                    String savedPassword = "";

                    if (loadSavedCredentials(savedSSID, savedPassword)) {
                        // If in AP mode only, switch to AP+STA mode for reconnection
                        wifi_mode_t currentMode = WiFi.getMode();
                        if (currentMode == WIFI_AP) {
                            Serial.println("[AUTO-RECONNECT] Switching to AP+STA mode...");
                            WiFi.mode(WIFI_AP_STA);
                            delay(100);
                        }

                        // Force disconnect and try again
                        WiFi.disconnect(true);
                        delay(100);

                        WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
                        autoReconnectStartTime = currentMillis;
                    } else {
                        autoReconnecting = false;
                    }
                } else {
                    // Show progress
                    Serial.print(".");
                }
            }
        } else {
            // Connected successfully
            if (autoReconnecting) {
                Serial.println("\n[AUTO-RECONNECT] Successfully reconnected!");
                Serial.print("[AUTO-RECONNECT] IP address: ");
                Serial.println(WiFi.localIP());
                autoReconnecting = false;
                autoReconnectStartTime = 0;
            }
        }
    }
}

void WiFiManager::switchToSTAMode() {
    Serial.println("[MODE] Switching from AP mode to STA mode...");

    // Stop the Access Point
    WiFi.softAPdisconnect(true);

    // Switch to Station mode only
    WiFi.mode(WIFI_STA);

    Serial.println("[MODE] Successfully switched to STA mode");
    Serial.print("[MODE] IP address: ");
    Serial.println(WiFi.localIP());
}

void WiFiManager::switchToAPMode() {
    Serial.println("[MODE] Switching from STA mode to AP mode...");

    // Disconnect from WiFi if connected
    WiFi.disconnect(true);
    delay(100);

    // Switch to AP mode and restart the AP
    WiFi.mode(WIFI_AP);

    String apSSID = "ppiot-" + getMacLastDigits();
    WiFi.softAP(apSSID.c_str(), AP_PASSWORD);

    IPAddress IP = WiFi.softAPIP();
    Serial.println("[MODE] Successfully switched to AP mode");
    Serial.print("[MODE] AP SSID: ");
    Serial.println(apSSID);
    Serial.print("[MODE] AP IP address: ");
    Serial.println(IP);
}

String WiFiManager::getMacLastDigits() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[7];
    sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(macStr);
}
