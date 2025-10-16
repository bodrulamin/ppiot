#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

const int LED_PIN = 2;
const int RESET_BUTTON_PIN = 0; // GPIO 0 - Usually the BOOT button on ESP32
const unsigned long RESET_HOLD_TIME = 10000; // 10 seconds in milliseconds

const char* AP_PASSWORD = "12345678"; // Minimum 8 characters for WPA2

Preferences preferences;
AsyncWebServer server(80);

String ssid = "";
String password = "";
bool isAPMode = false;

unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool scanInProgress = false;

// HTML for the configuration portal
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PPIOT WiFi Setup</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
            max-width: 400px;
            width: 90%;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: bold;
        }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 10px;
            padding-right: 45px;
            border: 2px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 16px;
        }
        input[type="text"]:focus, input[type="password"]:focus {
            outline: none;
            border-color: #667eea;
        }
        .password-wrapper {
            position: relative;
        }
        .toggle-password {
            position: absolute;
            right: 10px;
            top: 50%;
            transform: translateY(-50%);
            background: none;
            border: none;
            cursor: pointer;
            font-size: 18px;
            padding: 5px;
            color: #667eea;
            width: auto;
        }
        .toggle-password:hover {
            transform: translateY(-50%) scale(1.1);
        }
        select {
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 16px;
            background-color: white;
            cursor: pointer;
        }
        select:focus {
            outline: none;
            border-color: #667eea;
        }
        .refresh-btn {
            margin-top: 5px;
            padding: 8px 15px;
            background: #f0f0f0;
            border: 1px solid #ddd;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            color: #555;
            width: auto;
        }
        .refresh-btn:hover {
            background: #e0e0e0;
            transform: none;
        }
        .loading {
            color: #667eea;
            font-size: 14px;
            margin-top: 5px;
        }
        button {
            width: 100%;
            padding: 12px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
        }
        button:active {
            transform: translateY(0);
        }
        .info {
            background: #e7f3ff;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            text-align: center;
            color: #0066cc;
        }
        .status {
            text-align: center;
            margin-top: 15px;
            color: #28a745;
            font-weight: bold;
            display: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌐 PPIOT Setup</h1>
        <div class="info">
            Enter your WiFi credentials to connect this device to your network
        </div>
        <form id="wifiForm">
            <div class="form-group">
                <label for="ssid">Select WiFi Network:</label>
                <select id="ssid" name="ssid" required>
                    <option value="">Loading networks...</option>
                </select>
                <button type="button" class="refresh-btn" onclick="scanNetworks()">🔄 Refresh</button>
                <div class="loading" id="loading" style="display:none;">Scanning...</div>
            </div>
            <div class="form-group">
                <label for="password">WiFi Password:</label>
                <div class="password-wrapper">
                    <input type="password" id="password" name="password" required placeholder="Enter WiFi password">
                    <button type="button" class="toggle-password" onclick="togglePassword()">👁️</button>
                </div>
            </div>
            <button type="submit">Save & Connect</button>
        </form>
        <div class="status" id="status"></div>
    </div>
    <script>
        function getSignalIcon(rssi) {
            if (rssi > -50) return '📶';
            if (rssi > -60) return '📶';
            if (rssi > -70) return '📡';
            return '📉';
        }

        function scanNetworks() {
            var loading = document.getElementById('loading');
            var select = document.getElementById('ssid');
            var refreshBtn = document.querySelector('.refresh-btn');

            loading.style.display = 'block';
            refreshBtn.disabled = true;
            select.disabled = true;
            select.innerHTML = '<option value="">Scanning...</option>';

            // Start scanning on the server first
            var scanStart = new XMLHttpRequest();
            scanStart.open('GET', '/scan', true);
            scanStart.send();

            // Poll for results
            var pollAttempts = 0;
            var maxPolls = 10;

            function pollForResults() {
                pollAttempts++;

                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/scan', true);
                xhr.onload = function() {
                    if (xhr.status === 200) {
                        var networks = JSON.parse(xhr.responseText);

                        // If no networks yet and we haven't exceeded max polls, try again
                        if (networks.length === 0 && pollAttempts < maxPolls) {
                            setTimeout(pollForResults, 1000);
                            return;
                        }

                        // Done scanning
                        loading.style.display = 'none';
                        refreshBtn.disabled = false;
                        select.disabled = false;

                        select.innerHTML = '<option value="">-- Select Network --</option>';

                        if (networks.length === 0) {
                            select.innerHTML = '<option value="">No networks found - Try Refresh</option>';
                        } else {
                            networks.forEach(function(network) {
                                if (network.ssid && network.ssid.trim() !== '') {
                                    var option = document.createElement('option');
                                    option.value = network.ssid;
                                    var icon = getSignalIcon(network.rssi);
                                    var lock = network.secure !== 0 ? '🔒' : '🔓';
                                    option.textContent = icon + ' ' + network.ssid + ' ' + lock;
                                    select.appendChild(option);
                                }
                            });
                        }
                    } else {
                        loading.style.display = 'none';
                        refreshBtn.disabled = false;
                        select.disabled = false;
                        select.innerHTML = '<option value="">Error scanning - Try Refresh</option>';
                    }
                };
                xhr.onerror = function() {
                    loading.style.display = 'none';
                    refreshBtn.disabled = false;
                    select.disabled = false;
                    select.innerHTML = '<option value="">Connection error - Try Refresh</option>';
                };
                xhr.send();
            }

            // Start polling after initial delay
            setTimeout(pollForResults, 2000);
        }

        function togglePassword() {
            var passwordInput = document.getElementById('password');
            var toggleBtn = document.querySelector('.toggle-password');

            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                toggleBtn.textContent = '🙈';
            } else {
                passwordInput.type = 'password';
                toggleBtn.textContent = '👁️';
            }
        }

        // Load networks on page load
        window.addEventListener('load', function() {
            scanNetworks();
        });

        document.getElementById('wifiForm').addEventListener('submit', function(e) {
            e.preventDefault();
            var ssid = document.getElementById('ssid').value;
            var password = document.getElementById('password').value;

            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/save', true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.onload = function() {
                var status = document.getElementById('status');
                if (xhr.status === 200) {
                    status.style.display = 'block';
                    status.style.color = '#28a745';
                    status.textContent = 'Connected successfully! Device will restart and connect to WiFi...';
                } else {
                    status.style.display = 'block';
                    status.style.color = '#dc3545';
                    if (xhr.status === 400) {
                        status.textContent = xhr.responseText;
                    } else {
                        status.textContent = 'Error: Could not save configuration!';
                    }
                }
            };
            xhr.send('ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password));
        });
    </script>
</body>
</html>
)rawliteral";

// Function to get MAC address last 6 digits
String getMacLastDigits() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[7];
  sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
  return String(macStr);
}

// Start Access Point Mode
void startAPMode() {
  isAPMode = true;
  String apSSID = "ppiot-" + getMacLastDigits();

  Serial.println("Starting Access Point Mode");
  Serial.print("AP SSID: ");
  Serial.println(apSSID);
  Serial.print("AP Password: ");
  Serial.println(AP_PASSWORD);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID.c_str(), AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Setup web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
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
          // Log each network found
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

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String newSSID = request->getParam("ssid", true)->value();
      String newPassword = request->getParam("password", true)->value();

      Serial.println("Received WiFi credentials:");
      Serial.print("SSID: ");
      Serial.println(newSSID);

      // Try to connect to the new WiFi first
      Serial.println("Testing connection to new WiFi...");
      WiFi.mode(WIFI_AP_STA); // Keep AP running while testing
      WiFi.begin(newSSID.c_str(), newPassword.c_str());

      int attempts = 0;
      bool connected = false;
      while (attempts < 20 && !connected) {
        delay(500);
        Serial.print(".");
        if (WiFi.status() == WL_CONNECTED) {
          connected = true;
          Serial.println("\nSuccessfully connected to new WiFi!");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
        }
        attempts++;
      }

      if (connected) {
        // Only save credentials if connection was successful
        preferences.begin("wifi", false);
        preferences.putString("ssid", newSSID);
        preferences.putString("password", newPassword);
        preferences.end();

        request->send(200, "text/plain", "OK");
        Serial.println("New credentials saved. Restarting...");
        delay(1000);
        ESP.restart();
      } else {
        // Connection failed, keep old credentials
        Serial.println("\nFailed to connect to new WiFi. Keeping old credentials.");
        WiFi.disconnect();
        request->send(400, "text/plain", "Failed to connect to WiFi. Please check credentials and try again.");
      }
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  server.begin();
}

// Connect to WiFi using saved credentials
bool connectToWiFi() {
  if (ssid.length() == 0) {
    return false;
  }

  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // Use internal pullup resistor

  delay(1000);
  Serial.println("\n\n=== PPIOT Device Starting ===");

  // Check if reset button is held during boot
  Serial.println("Checking for factory reset button press...");
  unsigned long resetCheckStart = millis();
  bool factoryReset = false;

  // Check if button is held for 10 seconds at startup
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
      preferences.begin("wifi", false);
      preferences.clear();
      preferences.end();

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
    startAPMode();
    return;
  }

  // Try to load saved credentials
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid.length() > 0) {
    Serial.println("Found saved WiFi credentials");
    if (connectToWiFi()) {
      isAPMode = false;
      Serial.println("Device is in Station Mode");
    } else {
      Serial.println("Failed to connect to saved WiFi");
      Serial.println("Keeping credentials and starting AP mode for reconfiguration");
      // Do NOT clear credentials - keep them for user reference
      // User can either:
      // 1. Try again later (WiFi might be down temporarily)
      // 2. Reconfigure via web portal
      // 3. Factory reset if needed
      startAPMode();
    }
  } else {
    Serial.println("No saved credentials found");
    startAPMode();
  }
}

void loop() {
  // Blink LED only when in AP mode (hotspot active)
  if (isAPMode) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  } else {
    // Keep LED off when connected to WiFi
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}