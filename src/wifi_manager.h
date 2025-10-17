#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>

class WiFiManager {
private:
    Preferences preferences;
    String ssid;
    String password;
    bool hasStoredCredentials;

    // Auto-reconnect variables
    unsigned long lastWiFiCheck;
    bool autoReconnecting;
    unsigned long autoReconnectStartTime;

public:
    WiFiManager();

    void begin();
    bool loadSavedCredentials(String &savedSSID, String &savedPassword);
    bool saveCredentials(const String &newSSID, const String &newPassword);
    void clearCredentials();

    bool connectToWiFi();
    void handleAutoReconnect();

    String getSSID() const { return ssid; }
    String getPassword() const { return password; }
    bool hasCredentials() const { return hasStoredCredentials; }
    bool isAutoReconnecting() const { return autoReconnecting; }

    static String getMacLastDigits();
};

#endif // WIFI_MANAGER_H
