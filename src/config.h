#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
constexpr int LED_PIN = 2;
constexpr int RESET_BUTTON_PIN = 0; // GPIO 0 - Usually the BOOT button on ESP32
constexpr int TEMP_SENSOR_PIN = 4; // DHT22 temperature & humidity sensor pin
constexpr int DS18B20_PIN = 15; // DS18B20 temperature sensor pin

// Timing constants
constexpr unsigned long RESET_HOLD_TIME = 5000; // 5 seconds in milliseconds
constexpr unsigned long WIFI_CHECK_INTERVAL = 10000; // Check every 10 seconds
constexpr unsigned long AUTO_RECONNECT_TIMEOUT = 30000; // 30 seconds timeout
constexpr unsigned long LED_BLINK_INTERVAL = 500; // 500ms blink interval
constexpr unsigned long TEMP_READ_INTERVAL = 2000; // Read temperature every 2 seconds

// WiFi Access Point settings
constexpr char AP_PASSWORD[] = "12345678"; // Minimum 8 characters for WPA2

// MQTT Settings
#define MQTT_SERVER "itbir.com"
#define MQTT_PORT 1883
#define MQTT_USER "" // Leave empty if no authentication required
#define MQTT_PASSWORD "" // Leave empty if no authentication required
#define MQTT_PUBLISH_INTERVAL 2000 // Publish sensor data every 5 seconds

#endif // CONFIG_H
