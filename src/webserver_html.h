#ifndef WEBSERVER_HTML_H
#define WEBSERVER_HTML_H

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
            min-height: 100vh;
            margin: 0;
            padding: 15px;
        }
        .container {
            background: white;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
            max-width: 450px;
            width: 100%;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 25px;
            font-size: clamp(1.5rem, 5vw, 2rem);
        }
        .form-group {
            margin-bottom: 18px;
        }
        label {
            display: block;
            margin-bottom: 6px;
            color: #555;
            font-weight: bold;
            font-size: clamp(0.9rem, 3vw, 1rem);
        }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 12px;
            padding-right: 45px;
            border: 2px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 16px;
            -webkit-appearance: none;
            appearance: none;
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
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 16px;
            background-color: white;
            cursor: pointer;
            -webkit-appearance: none;
            appearance: none;
            background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 12 12'%3E%3Cpath fill='%23333' d='M6 9L1 4h10z'/%3E%3C/svg%3E");
            background-repeat: no-repeat;
            background-position: right 12px center;
            padding-right: 35px;
        }
        select:focus {
            outline: none;
            border-color: #667eea;
        }
        .refresh-btn {
            margin-top: 8px;
            padding: 10px 18px;
            background: #f0f0f0;
            border: 1px solid #ddd;
            border-radius: 5px;
            cursor: pointer;
            font-size: clamp(0.85rem, 3vw, 0.95rem);
            color: #555;
            width: auto;
            touch-action: manipulation;
        }
        .refresh-btn:hover, .refresh-btn:active {
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
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 5px;
            font-size: clamp(0.95rem, 3vw, 1.05rem);
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.2s;
            touch-action: manipulation;
            -webkit-tap-highlight-color: transparent;
        }
        button:hover {
            transform: translateY(-2px);
        }
        button:active {
            transform: translateY(0);
        }
        .info {
            background: #e7f3ff;
            padding: 12px;
            border-radius: 5px;
            margin-bottom: 20px;
            text-align: center;
            color: #0066cc;
            font-size: clamp(0.85rem, 3vw, 0.95rem);
        }
        .status {
            text-align: center;
            margin-top: 15px;
            color: #28a745;
            font-weight: bold;
            display: none;
            font-size: clamp(0.9rem, 3vw, 1rem);
        }
        .connected-info {
            background: #d4edda;
            border: 2px solid #28a745;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
            display: none;
        }
        .connected-info h3 {
            color: #155724;
            margin-bottom: 15px;
            font-size: clamp(1.1rem, 4vw, 1.3rem);
            text-align: center;
        }
        .info-row {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            padding: 8px;
            background: white;
            border-radius: 5px;
        }
        .info-label {
            font-weight: bold;
            color: #155724;
        }
        .info-value {
            color: #333;
            font-family: monospace;
        }
        .action-buttons {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-top: 15px;
        }
        .disconnect-btn {
            background: #ffc107;
            color: #000;
        }
        .disconnect-btn:hover {
            background: #e0a800;
        }
        .clear-btn {
            background: #dc3545;
            color: white;
        }
        .clear-btn:hover {
            background: #c82333;
        }

        /* Mobile optimizations */
        @media (max-width: 600px) {
            body {
                padding: 10px;
            }
            .container {
                padding: 20px;
            }
            h1 {
                margin-bottom: 20px;
            }
            .form-group {
                margin-bottom: 15px;
            }
            .saved-info {
                padding: 12px;
            }
            .divider {
                margin: 15px 0;
            }
        }

        /* Landscape mobile */
        @media (max-width: 900px) and (orientation: landscape) {
            body {
                padding: 10px;
                align-items: flex-start;
            }
            .container {
                margin: 10px 0;
            }
        }

        /* Larger screens */
        @media (min-width: 768px) {
            .container {
                padding: 30px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌐 PPIOT Setup</h1>
        <div class="info">
            Enter your WiFi credentials to connect this device to your network
        </div>

        <div id="connectedInfo" class="connected-info" style="display:none;">
            <h3>✅ WiFi Connected</h3>
            <div class="info-row">
                <span class="info-label">Network:</span>
                <span class="info-value" id="connectedSSID"></span>
            </div>
            <div class="info-row">
                <span class="info-label">IP Address:</span>
                <span class="info-value" id="ipAddress"></span>
            </div>
            <div class="action-buttons">
                <button type="button" class="disconnect-btn" onclick="disconnectWiFi()">
                    🔌 Disconnect
                </button>
                <button type="button" class="clear-btn" onclick="clearCredentials()">
                    🗑️ Clear Credentials
                </button>
            </div>
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
        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 10px; margin-top: 20px;">
            <a href="/dashboard" style="display: block; padding: 10px; background: #f0f0f0; border-radius: 5px; text-decoration: none; color: #667eea; font-weight: bold; text-align: center;">📊 Dashboard</a>
            <a href="/device" style="display: block; padding: 10px; background: #f0f0f0; border-radius: 5px; text-decoration: none; color: #667eea; font-weight: bold; text-align: center;">🖥️ Device Info</a>
        </div>
    </div>
    <script>
        function loadSavedInfo() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/info', true);
            xhr.onload = function() {
                if (xhr.status === 200) {
                    var info = JSON.parse(xhr.responseText);

                    // Check if connected
                    if (info.connected) {
                        // Show connected info
                        document.getElementById('connectedInfo').style.display = 'block';
                        document.getElementById('connectedSSID').textContent = info.current_ssid;
                        document.getElementById('ipAddress').textContent = info.ip_address;

                        // Hide saved credentials retry section
                        document.getElementById('savedCredentials').style.display = 'none';
                    } else if (info.has_saved) {
                        // Not connected but has saved credentials - show retry
                        document.getElementById('savedCredentials').style.display = 'block';
                        document.getElementById('savedSSID').textContent = info.saved_ssid;
                        document.getElementById('connectedInfo').style.display = 'none';
                    } else {
                        // No connection and no saved credentials
                        document.getElementById('connectedInfo').style.display = 'none';
                        document.getElementById('savedCredentials').style.display = 'none';
                    }
                }
            };
            xhr.send();
        }

        function disconnectWiFi() {
            if (!confirm('Are you sure you want to disconnect from WiFi?')) {
                return;
            }

            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/disconnect', true);
            xhr.onload = function() {
                if (xhr.status === 200) {
                    alert('Disconnected from WiFi');
                    location.reload();
                } else {
                    alert('Failed to disconnect');
                }
            };
            xhr.send();
        }

        function clearCredentials() {
            if (!confirm('Are you sure you want to clear saved WiFi credentials? This will disconnect the device.')) {
                return;
            }

            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/clear', true);
            xhr.onload = function() {
                if (xhr.status === 200) {
                    alert('Credentials cleared successfully');
                    location.reload();
                } else {
                    alert('Failed to clear credentials');
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
            var status = document.getElementById('status');

            // Show waiting message
            status.style.display = 'block';
            status.style.color = '#17a2b8';
            status.style.fontSize = '16px';
            status.style.fontWeight = 'bold';
            status.textContent = 'Connecting to WiFi... Please wait.';

            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/save', true);
            xhr.timeout = 15000; // 15 second timeout
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.onload = function() {
                if (xhr.status === 200) {
                    status.style.display = 'block';
                    status.style.color = '#28a745';
                    status.style.fontSize = '16px';
                    status.style.fontWeight = 'bold';
                    status.textContent = '✅ ' + xhr.responseText;
                } else {
                    status.style.display = 'block';
                    status.style.color = '#dc3545';
                    status.style.fontSize = '16px';
                    if (xhr.status === 400) {
                        status.textContent = '❌ ' + xhr.responseText;
                    } else {
                        status.textContent = '❌ Error: Could not save configuration!';
                    }
                }
            };
            xhr.onerror = function() {
                status.style.display = 'block';
                status.style.color = '#dc3545';
                status.style.fontSize = '16px';
                status.textContent = '❌ Connection error - Please try again';
            };
            xhr.ontimeout = function() {
                status.style.display = 'block';
                status.style.color = '#dc3545';
                status.style.fontSize = '16px';
                status.textContent = '❌ Request timed out - Please try again';
            };
            xhr.send('ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password));
        });
    </script>
</body>
</html>
)rawliteral";

#endif // WEBSERVER_HTML_H
