#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
constexpr int LED_PIN = 2;
constexpr int RESET_BUTTON_PIN = 0; // GPIO 0 - Usually the BOOT button on ESP32
constexpr int TEMP_SENSOR_PIN = 4; // DHT22 temperature & humidity sensor pin

// Timing constants
constexpr unsigned long RESET_HOLD_TIME = 5000; // 5 seconds in milliseconds
constexpr unsigned long WIFI_CHECK_INTERVAL = 10000; // Check every 10 seconds
constexpr unsigned long AUTO_RECONNECT_TIMEOUT = 30000; // 30 seconds timeout
constexpr unsigned long LED_BLINK_INTERVAL = 500; // 500ms blink interval
constexpr unsigned long TEMP_READ_INTERVAL = 2000; // Read temperature every 2 seconds

// WiFi Access Point settings
constexpr char AP_PASSWORD[] = "12345678"; // Minimum 8 characters for WPA2

#endif // CONFIG_H
