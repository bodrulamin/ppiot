#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
const int LED_PIN = 2;
const int RESET_BUTTON_PIN = 0; // GPIO 0 - Usually the BOOT button on ESP32
const int TEMP_SENSOR_PIN = 15; // Temperature sensor pin

// Timing constants
const unsigned long RESET_HOLD_TIME = 5000; // 5 seconds in milliseconds
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Check every 10 seconds
const unsigned long AUTO_RECONNECT_TIMEOUT = 30000; // 30 seconds timeout
const unsigned long LED_BLINK_INTERVAL = 500; // 500ms blink interval
const unsigned long TEMP_READ_INTERVAL = 2000; // Read temperature every 2 seconds

// WiFi Access Point settings
const char* AP_PASSWORD = "12345678"; // Minimum 8 characters for WPA2

#endif // CONFIG_H
