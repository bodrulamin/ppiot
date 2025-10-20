#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

// HTML for the temperature/humidity dashboard
const char dashboard_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PPIOT Dashboard</title>
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
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 10px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 800px;
            width: 100%;
            padding: 20px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            font-size: clamp(1.5rem, 5vw, 2.5rem);
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: clamp(0.9rem, 3vw, 1.1rem);
        }
        .cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        .card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border-radius: 15px;
            padding: 20px 15px;
            text-align: center;
            color: white;
            box-shadow: 0 10px 25px rgba(102, 126, 234, 0.3);
            transition: transform 0.3s ease;
            min-height: 150px;
            display: flex;
            flex-direction: column;
            justify-content: center;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card:active {
            transform: translateY(-2px);
        }
        .card-icon {
            font-size: clamp(2em, 8vw, 3em);
            margin-bottom: 8px;
        }
        .card-label {
            font-size: clamp(0.75rem, 2.5vw, 0.9rem);
            opacity: 0.9;
            margin-bottom: 8px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .card-value {
            font-size: clamp(1.8rem, 6vw, 2.5rem);
            font-weight: bold;
            margin-bottom: 5px;
        }
        .card-unit {
            font-size: clamp(1rem, 3vw, 1.2rem);
            opacity: 0.8;
        }
        .humidity-card {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
        }
        .heat-index-card {
            background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
        }
        .status {
            text-align: center;
            padding: 12px;
            border-radius: 10px;
            margin-bottom: 20px;
            font-weight: bold;
            font-size: clamp(0.85rem, 3vw, 1rem);
        }
        .status.online {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .status.offline {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .last-update {
            text-align: center;
            color: #666;
            font-size: 0.9em;
            margin-top: 20px;
        }
        .refresh-btn {
            display: flex; /* Use flex to align content */
            align-items: center; /* Vertically center content */
            justify-content: center; /* Horizontally center content */
            margin: 20px auto 0;
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
        
        /* New loading indicator style */
        .loading-icon {
            display: none; /* Hidden by default */
            margin-left: 10px;
            width: 1em; /* Set width relative to font size */
            height: 1em; /* Set height relative to font size */
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-top: 2px solid white;
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .nav-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            padding: 12px;
            background: #f8f9fa;
            border-radius: 10px;
            text-decoration: none;
            color: #667eea;
            font-weight: bold;
            font-size: clamp(0.9rem, 3vw, 1rem);
            transition: background 0.3s;
            touch-action: manipulation;
        }
        .nav-link:hover, .nav-link:active {
            background: #e9ecef;
        }

        /* Mobile optimizations */
        @media (max-width: 600px) {
            .container {
                padding: 15px;
                border-radius: 15px;
            }
            .cards {
                gap: 10px;
            }
            .card {
                padding: 15px 10px;
                min-height: 130px;
            }
            h1 {
                margin-bottom: 5px;
            }
            .subtitle {
                margin-bottom: 20px;
            }
        }

        /* Landscape mobile */
        @media (max-width: 900px) and (orientation: landscape) {
            body {
                padding: 10px;
            }
            .container {
                padding: 15px;
            }
            .cards {
                grid-template-columns: repeat(3, 1fr);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌡️ PPIOT Dashboard</h1>
        <p class="subtitle">Real-time Temperature & Humidity Monitor</p>

        <div id="statusDHT22" class="status online">
            📡 DHT22 Sensor Online
        </div>

        <div class="cards">
            <div class="card">
                <div class="card-icon">🌡️</div>
                <div class="card-label">Temperature (DHT22)</div>
                <div class="card-value" id="temperature">--</div>
                <div class="card-unit">°C</div>
            </div>

            <div class="card humidity-card">
                <div class="card-icon">💧</div>
                <div class="card-label">Humidity</div>
                <div class="card-value" id="humidity">--</div>
                <div class="card-unit">%</div>
            </div>

            <div class="card heat-index-card">
                <div class="card-icon">🔥</div>
                <div class="card-label">Heat Index</div>
                <div class="card-value" id="heatIndex">--</div>
                <div class="card-unit">°C</div>
            </div>
        </div>

        <div id="statusDS18B20" class="status online" style="margin-top: 20px;">
            📡 DS18B20 Sensor Online
        </div>

        <div class="cards">
            <div class="card" style="background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);">
                <div class="card-icon">🌡️</div>
                <div class="card-label">Temperature (DS18B20)</div>
                <div class="card-value" id="temperatureDS18B20">--</div>
                <div class="card-unit">°C</div>
            </div>
        </div>

        <div class="last-update" id="lastUpdate">
            Last updated: Never
        </div>

        <button class="refresh-btn" onclick="refreshData()">
            <span id="refreshText">🔄 Refresh Data</span>
            <div id="loadingIcon" class="loading-icon"></div>
        </button>

        <div class="nav-links" style="display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 10px; margin-top: 20px;">
            <a href="/" class="nav-link">⚙️ WiFi Config</a>
            <a href="/device" class="nav-link">🖥️ Device Info</a>
        </div>
    </div>

    <script>
        let autoRefreshInterval;

        function updateData(data) {
            const statusDHT22 = document.getElementById('statusDHT22');
            const statusDS18B20 = document.getElementById('statusDS18B20');
            const tempElement = document.getElementById('temperature');
            const humidityElement = document.getElementById('humidity');
            const heatIndexElement = document.getElementById('heatIndex');
            const tempDS18B20Element = document.getElementById('temperatureDS18B20');
            const lastUpdate = document.getElementById('lastUpdate');

            // Update DHT22 data
            if (data.dht22 && data.dht22.valid) {
                tempElement.textContent = data.dht22.temperature.toFixed(1);
                humidityElement.textContent = data.dht22.humidity.toFixed(1);
                heatIndexElement.textContent = data.dht22.heatIndex.toFixed(1);

                statusDHT22.className = 'status online';
                statusDHT22.textContent = '📡 DHT22 Sensor Online';
            } else {
                tempElement.textContent = '--';
                humidityElement.textContent = '--';
                heatIndexElement.textContent = '--';

                statusDHT22.className = 'status offline';
                statusDHT22.textContent = '⚠️ DHT22 Sensor Offline';
            }

            // Update DS18B20 data
            if (data.ds18b20 && data.ds18b20.valid) {
                tempDS18B20Element.textContent = data.ds18b20.temperature.toFixed(1);

                statusDS18B20.className = 'status online';
                statusDS18B20.textContent = '📡 DS18B20 Sensor Online';
            } else {
                tempDS18B20Element.textContent = '--';

                statusDS18B20.className = 'status offline';
                statusDS18B20.textContent = '⚠️ DS18B20 Sensor Offline';
            }

            // Update last update time
            const now = new Date();
            lastUpdate.textContent = 'Last updated: ' + now.toLocaleTimeString('en-US', { timeZone: 'Asia/Dhaka' });
        }

        function refreshData() {
            const refreshButton = document.querySelector('.refresh-btn');
            const refreshText = document.getElementById('refreshText');
            const loadingIcon = document.getElementById('loadingIcon');

            // 1. Show Loading State
            refreshButton.disabled = true; // Prevent multiple clicks
            loadingIcon.style.display = 'block';

            fetch('/api/sensor')
                .then(response => response.json())
                .then(data => {
                    updateData(data);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    const statusDHT22 = document.getElementById('statusDHT22');
                    const statusDS18B20 = document.getElementById('statusDS18B20');
                    statusDHT22.className = 'status offline';
                    statusDHT22.textContent = '❌ Connection Error';
                    statusDS18B20.className = 'status offline';
                    statusDS18B20.textContent = '❌ Connection Error';
                })
                .finally(() => {
                    // 2. Hide Loading State regardless of success or failure
                    refreshButton.disabled = false;
                    loadingIcon.style.display = 'none';
                    refreshText.textContent = '🔄 Refresh Data';
                });
        }

        // Auto-refresh every 5 seconds
        function startAutoRefresh() {
            refreshData(); // Initial load
            autoRefreshInterval = setInterval(refreshData, 5000);
        }

        // Stop auto-refresh when page is not visible
        document.addEventListener('visibilitychange', function() {
            if (document.hidden) {
                clearInterval(autoRefreshInterval);
            } else {
                startAutoRefresh();
            }
        });

        // Start auto-refresh when page loads
        window.addEventListener('load', startAutoRefresh);
    </script>
</body>
</html>
)rawliteral";

#endif // DASHBOARD_HTML_H
