#ifndef DEVICEINFO_HTML_H
#define DEVICEINFO_HTML_H

// HTML for the device information page
const char deviceinfo_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PPIOT Device Info</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 15px;
        }
        .container {
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 900px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            font-size: clamp(1.5rem, 5vw, 2rem);
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 25px;
            font-size: clamp(0.85rem, 3vw, 1rem);
        }
        .section {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .section h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: clamp(1.1rem, 4vw, 1.4rem);
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 12px;
        }
        .info-item {
            background: white;
            padding: 12px 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
            display: flex;
            flex-direction: column;
            gap: 5px;
        }
        .info-label {
            font-size: clamp(0.75rem, 2.5vw, 0.85rem);
            color: #666;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            font-weight: 600;
        }
        .info-value {
            font-size: clamp(1rem, 3.5vw, 1.2rem);
            color: #333;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            word-break: break-all;
        }
        .progress-bar {
            background: #e9ecef;
            border-radius: 10px;
            height: 25px;
            overflow: hidden;
            margin-top: 8px;
            position: relative;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            transition: width 0.5s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-size: 0.8rem;
            font-weight: bold;
        }
        .progress-text {
            position: absolute;
            width: 100%;
            text-align: center;
            line-height: 25px;
            font-size: 0.8rem;
            font-weight: bold;
            color: #333;
            z-index: 1;
        }
        .status-badge {
            display: inline-block;
            padding: 5px 12px;
            border-radius: 15px;
            font-size: 0.85rem;
            font-weight: bold;
        }
        .status-online {
            background: #d4edda;
            color: #155724;
        }
        .status-offline {
            background: #f8d7da;
            color: #721c24;
        }
        .refresh-btn {
            display: block;
            margin: 20px auto;
            padding: 12px 30px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 25px;
            font-size: clamp(0.9rem, 3vw, 1rem);
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.2s;
            width: 100%;
            max-width: 300px;
            touch-action: manipulation;
        }
        .refresh-btn:hover {
            transform: scale(1.05);
        }
        .refresh-btn:active {
            transform: scale(0.95);
        }
        .nav-links {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
            margin-top: 20px;
        }
        .nav-link {
            display: block;
            text-align: center;
            padding: 12px;
            background: #f8f9fa;
            border-radius: 10px;
            text-decoration: none;
            color: #667eea;
            font-weight: bold;
            font-size: clamp(0.85rem, 3vw, 0.95rem);
            transition: background 0.3s;
            touch-action: manipulation;
        }
        .nav-link:hover, .nav-link:active {
            background: #e9ecef;
        }
        .loading {
            text-align: center;
            padding: 20px;
            color: #667eea;
        }
        .spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto 10px;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        @media (max-width: 600px) {
            .container {
                padding: 15px;
            }
            .section {
                padding: 15px;
            }
            .info-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🖥️ PPIOT Device Info</h1>
        <p class="subtitle">System Information & Statistics</p>

        <div id="loading" class="loading">
            <div class="spinner"></div>
            <p>Loading device information...</p>
        </div>

        <div id="content" style="display:none;">
            <!-- System Information -->
            <div class="section">
                <h2>💻 System Information</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Chip Model</div>
                        <div class="info-value" id="chipModel">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">CPU Cores</div>
                        <div class="info-value" id="cpuCores">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">CPU Frequency</div>
                        <div class="info-value" id="cpuFreq">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">SDK Version</div>
                        <div class="info-value" id="sdkVersion">-</div>
                    </div>
                </div>
            </div>

            <!-- Memory Information -->
            <div class="section">
                <h2>💾 Memory (RAM)</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Total RAM</div>
                        <div class="info-value" id="totalRam">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Free RAM</div>
                        <div class="info-value" id="freeRam">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Used RAM</div>
                        <div class="info-value" id="usedRam">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Largest Free Block</div>
                        <div class="info-value" id="largestBlock">-</div>
                    </div>
                </div>
                <div class="progress-bar">
                    <div class="progress-text" id="ramPercentText">0%</div>
                    <div class="progress-fill" id="ramProgress" style="width: 0%"></div>
                </div>
            </div>

            <!-- Flash Memory -->
            <div class="section">
                <h2>💿 Flash Memory (ROM)</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Flash Size</div>
                        <div class="info-value" id="flashSize">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Flash Speed</div>
                        <div class="info-value" id="flashSpeed">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Flash Mode</div>
                        <div class="info-value" id="flashMode">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Sketch Size</div>
                        <div class="info-value" id="sketchSize">-</div>
                    </div>
                </div>
                <div class="progress-bar">
                    <div class="progress-text" id="flashPercentText">0%</div>
                    <div class="progress-fill" id="flashProgress" style="width: 0%"></div>
                </div>
            </div>

            <!-- WiFi Information -->
            <div class="section">
                <h2>📡 WiFi Information</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Status</div>
                        <div class="info-value" id="wifiStatus">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">SSID</div>
                        <div class="info-value" id="wifiSSID">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">IP Address</div>
                        <div class="info-value" id="ipAddress">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">MAC Address</div>
                        <div class="info-value" id="macAddress">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Signal Strength</div>
                        <div class="info-value" id="rssi">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Gateway</div>
                        <div class="info-value" id="gateway">-</div>
                    </div>
                </div>
            </div>

            <!-- Runtime Information -->
            <div class="section">
                <h2>⏱️ Runtime Information</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Uptime</div>
                        <div class="info-value" id="uptime">-</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Reset Reason</div>
                        <div class="info-value" id="resetReason">-</div>
                    </div>
                </div>
            </div>

            <button class="refresh-btn" onclick="loadDeviceInfo()">🔄 Refresh Data</button>

            <div class="nav-links">
                <a href="/" class="nav-link">⚙️ WiFi Config</a>
                <a href="/dashboard" class="nav-link">📊 Dashboard</a>
            </div>
        </div>
    </div>

    <script>
        function formatBytes(bytes) {
            if (bytes < 1024) return bytes + ' B';
            if (bytes < 1048576) return (bytes / 1024).toFixed(2) + ' KB';
            return (bytes / 1048576).toFixed(2) + ' MB';
        }

        function formatUptime(ms) {
            var seconds = Math.floor(ms / 1000);
            var minutes = Math.floor(seconds / 60);
            var hours = Math.floor(minutes / 60);
            var days = Math.floor(hours / 24);

            hours = hours % 24;
            minutes = minutes % 60;
            seconds = seconds % 60;

            if (days > 0) return days + 'd ' + hours + 'h ' + minutes + 'm';
            if (hours > 0) return hours + 'h ' + minutes + 'm ' + seconds + 's';
            if (minutes > 0) return minutes + 'm ' + seconds + 's';
            return seconds + 's';
        }

        function loadDeviceInfo() {
            document.getElementById('loading').style.display = 'block';
            document.getElementById('content').style.display = 'none';

            fetch('/api/device')
                .then(response => response.json())
                .then(data => {
                    // System Info
                    document.getElementById('chipModel').textContent = data.chip_model;
                    document.getElementById('cpuCores').textContent = data.cpu_cores;
                    document.getElementById('cpuFreq').textContent = data.cpu_freq + ' MHz';
                    document.getElementById('sdkVersion').textContent = data.sdk_version;

                    // RAM Info
                    document.getElementById('totalRam').textContent = formatBytes(data.total_ram);
                    document.getElementById('freeRam').textContent = formatBytes(data.free_ram);
                    document.getElementById('usedRam').textContent = formatBytes(data.used_ram);
                    document.getElementById('largestBlock').textContent = formatBytes(data.largest_block);

                    var ramPercent = ((data.used_ram / data.total_ram) * 100).toFixed(1);
                    document.getElementById('ramProgress').style.width = ramPercent + '%';
                    document.getElementById('ramPercentText').textContent = ramPercent + '% Used';

                    // Flash Info
                    document.getElementById('flashSize').textContent = formatBytes(data.flash_size);
                    document.getElementById('flashSpeed').textContent = (data.flash_speed / 1000000) + ' MHz';
                    document.getElementById('flashMode').textContent = data.flash_mode;
                    document.getElementById('sketchSize').textContent = formatBytes(data.sketch_size);

                    var flashPercent = ((data.sketch_size / data.flash_size) * 100).toFixed(1);
                    document.getElementById('flashProgress').style.width = flashPercent + '%';
                    document.getElementById('flashPercentText').textContent = flashPercent + '% Used';

                    // WiFi Info
                    var statusBadge = data.wifi_connected
                        ? '<span class="status-badge status-online">✅ Connected</span>'
                        : '<span class="status-badge status-offline">❌ Disconnected</span>';
                    document.getElementById('wifiStatus').innerHTML = statusBadge;
                    document.getElementById('wifiSSID').textContent = data.wifi_ssid || 'Not connected';
                    document.getElementById('ipAddress').textContent = data.ip_address || 'N/A';
                    document.getElementById('macAddress').textContent = data.mac_address;
                    document.getElementById('rssi').textContent = data.wifi_connected ? data.rssi + ' dBm' : 'N/A';
                    document.getElementById('gateway').textContent = data.gateway || 'N/A';

                    // Runtime Info
                    document.getElementById('uptime').textContent = formatUptime(data.uptime);
                    document.getElementById('resetReason').textContent = data.reset_reason;

                    document.getElementById('loading').style.display = 'none';
                    document.getElementById('content').style.display = 'block';
                })
                .catch(error => {
                    console.error('Error loading device info:', error);
                    document.getElementById('loading').innerHTML = '<p style="color: red;">Failed to load device information</p>';
                });
        }

        // Load on page load
        window.addEventListener('load', loadDeviceInfo);
    </script>
</body>
</html>
)rawliteral";

#endif // DEVICEINFO_HTML_H
