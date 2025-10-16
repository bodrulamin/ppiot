#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

const int LED_PIN = 2;
const int RESET_BUTTON_PIN = 0; // GPIO 0 - Usually the BOOT button on ESP32
const unsigned long RESET_HOLD_TIME = 5000; // 5 seconds in milliseconds

const char* AP_PASSWORD = "12345678"; // Minimum 8 characters for WPA2

Preferences preferences;
AsyncWebServer server(80);

String ssid = "";
String password = "";
bool isAPMode = false;
bool hasStoredCredentials = false; // Cache to avoid repeated NVS reads when empty

unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool scanInProgress = false;
bool retryInProgress = false;
unsigned long retryStartTime = 0;
AsyncWebServerRequest* retryRequest = nullptr;

// Save credentials variables
bool saveInProgress = false;
unsigned long saveStartTime = 0;
AsyncWebServerRequest* saveRequest = nullptr;
String newSSID = "";
String newPassword = "";

// Check credentials variables
bool checkInProgress = false;
unsigned long checkStartTime = 0;
AsyncWebServerRequest* checkRequest = nullptr;
String checkSSID = "";
String checkPassword = "";

// Auto-reconnect variables
unsigned long lastWiFiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Check every 10 seconds
bool autoReconnecting = false;
unsigned long autoReconnectStartTime = 0;
const unsigned long AUTO_RECONNECT_TIMEOUT = 30000; // 30 seconds timeout

// LED blink variables
unsigned long lastLEDToggle = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // 500ms blink interval
bool ledState = false;

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
        .saved-info {
            background: #fff3cd;
            border: 1px solid #ffc107;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            text-align: center;
        }
        .saved-info strong {
            color: #856404;
            display: block;
            margin-bottom: 10px;
        }
        .saved-ssid {
            color: #856404;
            font-weight: bold;
            font-size: 16px;
            margin: 10px 0;
        }
        .retry-btn {
            width: 100%;
            padding: 12px;
            background: #28a745;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            margin-bottom: 20px;
            transition: transform 0.2s;
        }
        .retry-btn:hover {
            background: #218838;
            transform: translateY(-2px);
        }
        .retry-btn:disabled {
            background: #6c757d;
            cursor: not-allowed;
            transform: none;
        }
        .check-btn {
            width: 100%;
            padding: 12px;
            background: #17a2b8;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            margin-bottom: 10px;
            transition: transform 0.2s;
        }
        .check-btn:hover {
            background: #138496;
            transform: translateY(-2px);
        }
        .check-btn:disabled {
            background: #6c757d;
            cursor: not-allowed;
            transform: none;
        }
        .divider {
            text-align: center;
            margin: 20px 0;
            position: relative;
        }
        .divider::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 0;
            right: 0;
            height: 1px;
            background: #ddd;
        }
        .divider span {
            background: white;
            padding: 0 15px;
            position: relative;
            color: #999;
            font-size: 14px;
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

        <div id="savedCredentials" style="display:none;">
            <div class="saved-info">
                <strong>Saved WiFi Configuration</strong>
                <div class="saved-ssid" id="savedSSID"></div>
                <button type="button" class="retry-btn" onclick="retryConnection()" id="retryBtn">
                    🔄 Retry Connection
                </button>
            </div>
            <div class="divider"><span>OR CONFIGURE NEW WIFI</span></div>
        </div>

        <form id="wifiForm">
            <div class="form-group">
                <label for="ssid">Select WiFi Network:</label>
                <select id="ssid" name="ssid" required>
                    <option value="">Click refresh to scan networks</option>
                </select>
                <button type="button" class="refresh-btn" onclick="scanNetworks()">🔄 Scan Networks</button>
                <div class="loading" id="loading" style="display:none;">Scanning...</div>
            </div>
            <div class="form-group">
                <label for="password">WiFi Password:</label>
                <div class="password-wrapper">
                    <input type="password" id="password" name="password" required placeholder="Enter WiFi password">
                    <button type="button" class="toggle-password" onclick="togglePassword()">👁️</button>
                </div>
            </div>
            <button type="button" class="check-btn" onclick="checkConnection()" id="checkBtn">🔍 Check Connection</button>
            <button type="submit">Save & Connect</button>
        </form>
        <div class="status" id="status"></div>
    </div>
    <script>
        function loadSavedInfo() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/info', true);
            xhr.onload = function() {
                if (xhr.status === 200) {
                    var info = JSON.parse(xhr.responseText);
                    if (info.has_saved) {
                        document.getElementById('savedCredentials').style.display = 'block';
                        document.getElementById('savedSSID').textContent = info.saved_ssid;
                    }
                }
            };
            xhr.send();
        }

        function checkConnection() {
            var ssid = document.getElementById('ssid').value;
            var password = document.getElementById('password').value;
            var checkBtn = document.getElementById('checkBtn');
            var status = document.getElementById('status');

            if (!ssid || !password) {
                status.style.display = 'block';
                status.style.color = '#dc3545';
                status.textContent = 'Please select a network and enter password';
                return;
            }

            checkBtn.disabled = true;
            checkBtn.textContent = 'Testing...';
            status.style.display = 'block';
            status.style.color = '#17a2b8';
            status.textContent = 'Testing connection... Please wait.';

            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/check', true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.onload = function() {
                checkBtn.disabled = false;
                checkBtn.textContent = '🔍 Check Connection';
                status.style.display = 'block';
                if (xhr.status === 200) {
                    status.style.color = '#28a745';
                    status.textContent = '✅ ' + xhr.responseText;
                } else {
                    status.style.color = '#dc3545';
                    status.textContent = '❌ ' + xhr.responseText;
                }
            };
            xhr.onerror = function() {
                checkBtn.disabled = false;
                checkBtn.textContent = '🔍 Check Connection';
                status.style.display = 'block';
                status.style.color = '#dc3545';
                status.textContent = 'Connection error - Please try again';
            };
            xhr.send('ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password));
        }

        function retryConnection() {
            var retryBtn = document.getElementById('retryBtn');
            var status = document.getElementById('status');

            retryBtn.disabled = true;
            retryBtn.textContent = 'Connecting...';
            status.style.display = 'none';

            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/retry', true);
            xhr.onload = function() {
                if (xhr.status === 200) {
                    status.style.display = 'block';
                    status.style.color = '#28a745';
                    status.textContent = xhr.responseText;
                } else {
                    retryBtn.disabled = false;
                    retryBtn.textContent = '🔄 Retry Connection';
                    status.style.display = 'block';
                    status.style.color = '#dc3545';
                    status.textContent = xhr.responseText;
                }
            };
            xhr.onerror = function() {
                retryBtn.disabled = false;
                retryBtn.textContent = '🔄 Retry Connection';
                status.style.display = 'block';
                status.style.color = '#dc3545';
                status.textContent = 'Connection error - Please try again';
            };
            xhr.send();
        }

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

        // Load saved info on page load (don't auto-scan networks)
        window.addEventListener('load', function() {
            loadSavedInfo();
            // User must click refresh button to scan networks
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
                    status.style.fontSize = '16px';
                    status.style.fontWeight = 'bold';
                    // Use the exact message from server
                    status.textContent = xhr.responseText;
                } else {
                    status.style.display = 'block';
                    status.style.color = '#dc3545';
                    status.style.fontSize = '16px';
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

// Helper function to load saved WiFi credentials
bool loadSavedCredentials(String &savedSSID, String &savedPassword) {
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

// Start Access Point Mode
void startAPMode() {
  isAPMode = true;
  String apSSID = "ppiot-" + getMacLastDigits();

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

  // Setup web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
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

  server.on("/retry", HTTP_GET, [](AsyncWebServerRequest *request){
    if (retryInProgress) {
      request->send(400, "text/plain", "Retry already in progress");
      return;
    }

    // Get saved credentials
    String savedSSID = "";
    String savedPassword = "";
    if (!loadSavedCredentials(savedSSID, savedPassword)) {
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

    // Don't send response yet - will be handled in loop()
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    // Return saved SSID (not password for security)
    String savedSSID = "";
    String savedPassword = "";
    bool hasCredentials = loadSavedCredentials(savedSSID, savedPassword);

    String json = "{";
    json += "\"saved_ssid\":\"" + savedSSID + "\",";
    json += "\"has_saved\":";
    json += hasCredentials ? "true" : "false";
    json += "}";

    request->send(200, "application/json", json);
  });

  server.on("/check", HTTP_POST, [](AsyncWebServerRequest *request){
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

      // Don't send response yet - will be handled in loop()
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
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

      // Send immediate acknowledgment to keep browser connection alive
      request->send(202, "text/plain", "Testing connection... Please wait.");

      // Try to connect to the new WiFi
      Serial.println("Testing connection to new WiFi...");

      // Small delay to ensure response is sent
      delay(100);

      // Already in AP+STA mode, just start WiFi connection
      WiFi.begin(newSSID.c_str(), newPassword.c_str());

      saveInProgress = true;
      saveStartTime = millis();
      saveRequest = nullptr; // No request to respond to later

      // Connection result will be handled in loop()
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

      // Clear cache flag
      hasStoredCredentials = false;

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

  // Try to load saved credentials (read directly from NVS on boot)
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
  }

  if (ssid.length() > 0) {
    Serial.println("Found saved WiFi credentials");

    // Always start AP mode first
    startAPMode();

    if (connectToWiFi()) {
      isAPMode = false;
      Serial.println("Device connected to WiFi - AP running in background");
    } else {
      Serial.println("Failed to connect to saved WiFi");
      Serial.println("AP visible for configuration, continuously trying to reconnect...");

      isAPMode = true;

      // Enable auto-reconnect for background connection attempts
      Serial.println("Auto-reconnect enabled - will keep trying in background");
      autoReconnecting = false; // Will be triggered by the loop
    }
  } else {
    Serial.println("No saved credentials found");
    startAPMode();
  }
}

void loop() {
  // Handle async save connection from web interface
  if (saveInProgress) {
    unsigned long elapsed = millis() - saveStartTime;

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nSuccessfully connected to new WiFi!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      // Save credentials
      preferences.begin("wifi", false);
      preferences.putString("ssid", newSSID);
      preferences.putString("password", newPassword);
      preferences.end();

      // Update cache flag since we now have stored credentials
      hasStoredCredentials = true;

      // Update global credentials
      ssid = newSSID;
      password = newPassword;

      saveInProgress = false;

      Serial.println("New credentials saved and connected successfully!");
      Serial.println("Device is now connected to WiFi. AP still running in background.");

      // Switch to connected mode (turn off AP mode LED blinking)
      isAPMode = false;
    } else if (elapsed > 10000) {
      // Timeout after 10 seconds
      Serial.println("\nFailed to connect to new WiFi. Keeping old credentials.");
      WiFi.disconnect();

      saveInProgress = false;
      Serial.println("Connection failed - please try again");
    }
    // Otherwise keep waiting (non-blocking)
    return;
  }

  // Handle async check connection from web interface
  if (checkInProgress && checkRequest != nullptr) {
    unsigned long elapsed = millis() - checkStartTime;

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[CHECK] Connection test successful!");
      Serial.print("[CHECK] IP address: ");
      Serial.println(WiFi.localIP());

      checkRequest->send(200, "text/plain", "Connection successful! Credentials are valid.");
      checkRequest = nullptr;
      checkInProgress = false;

      // Disconnect from test connection to return to original state
      WiFi.disconnect();
      delay(100);
    } else if (elapsed > 10000) {
      // Timeout after 10 seconds
      Serial.println("\n[CHECK] Connection test failed - timeout");

      checkRequest->send(400, "text/plain", "Connection failed. Please check credentials.");
      checkRequest = nullptr;
      checkInProgress = false;

      // Disconnect from failed attempt
      WiFi.disconnect();
    }
    // Otherwise keep waiting (non-blocking)
    return;
  }

  // Handle async retry connection from web interface
  if (retryInProgress && retryRequest != nullptr) {
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

      // Switch to connected mode (turn off AP mode LED blinking)
      isAPMode = false;
    } else if (elapsed > 10000) {
      // Timeout after 10 seconds
      Serial.println("\n[RETRY] Failed to reconnect - timeout");
      WiFi.disconnect();

      retryRequest->send(400, "text/plain", "Failed to connect. WiFi may be down or password changed.");
      retryRequest = nullptr;
      retryInProgress = false;
      autoReconnecting = false;
    }
    // Otherwise keep waiting (non-blocking)
    return;
  }

  // Auto-reconnect logic - works in both AP mode and station mode
  if (!retryInProgress) {
    unsigned long currentMillis = millis();

    // Check WiFi status periodically
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
      lastWiFiCheck = currentMillis;

      wl_status_t status = WiFi.status();

      // Check if WiFi is disconnected or blocked
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

            // Disconnect first to clear any stuck state
            WiFi.disconnect(true);
            delay(100);

            // Reconnect in AP+STA mode if AP is active

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

          // If we were in AP mode and successfully connected
          if (isAPMode) {
            Serial.println("[AUTO-RECONNECT] Connected! AP still running in background");
            isAPMode = false;
          }
        }
      }
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
}