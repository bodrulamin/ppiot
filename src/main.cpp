#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

const int LED_PIN = 2;
const char* AP_PASSWORD = "12345678"; // Minimum 8 characters for WPA2

Preferences preferences;
AsyncWebServer server(80);

String ssid = "";
String password = "";
bool isAPMode = false;

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
                <label for="ssid">WiFi SSID:</label>
                <input type="text" id="ssid" name="ssid" required placeholder="Enter WiFi name">
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
                    status.textContent = 'Configuration saved! Device will restart and connect to WiFi...';
                } else {
                    status.style.display = 'block';
                    status.style.color = '#dc3545';
                    status.textContent = 'Error saving configuration!';
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

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      Serial.println("Received WiFi credentials:");
      Serial.print("SSID: ");
      Serial.println(ssid);

      // Save to preferences
      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.end();

      request->send(200, "text/plain", "OK");

      delay(1000);
      ESP.restart();
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

  delay(1000);
  Serial.println("\n\n=== PPIOT Device Starting ===");

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
      Serial.println("Failed to connect, starting AP mode");
      // Clear invalid credentials
      preferences.begin("wifi", false);
      preferences.clear();
      preferences.end();
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