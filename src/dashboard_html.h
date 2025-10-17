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
            display: block;
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
        .loading {
            display: none;
            text-align: center;
            color: #667eea;
            margin: 20px 0;
        }
        .spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
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

        <div id="status" class="status online">
            📡 Sensor Online
        </div>

        <div class="cards">
            <div class="card">
                <div class="card-icon">🌡️</div>
                <div class="card-label">Temperature</div>
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

        <div class="loading" id="loading">
            <div class="spinner"></div>
            <p>Loading...</p>
        </div>

        <div class="last-update" id="lastUpdate">
            Last updated: Never
        </div>

        <button class="refresh-btn" onclick="refreshData()">🔄 Refresh Data</button>

        <a href="/" class="nav-link">⚙️ WiFi Configuration</a>
    </div>

    <script>
        let autoRefreshInterval;

        function updateData(data) {
            const status = document.getElementById('status');
            const tempElement = document.getElementById('temperature');
            const humidityElement = document.getElementById('humidity');
            const heatIndexElement = document.getElementById('heatIndex');
            const lastUpdate = document.getElementById('lastUpdate');

            if (data.valid) {
                tempElement.textContent = data.temperature.toFixed(1);
                humidityElement.textContent = data.humidity.toFixed(1);
                heatIndexElement.textContent = data.heatIndex.toFixed(1);

                status.className = 'status online';
                status.textContent = '📡 Sensor Online';

                const now = new Date();
                lastUpdate.textContent = 'Last updated: ' + now.toLocaleTimeString();
            } else {
                tempElement.textContent = '--';
                humidityElement.textContent = '--';
                heatIndexElement.textContent = '--';

                status.className = 'status offline';
                status.textContent = '⚠️ Sensor Offline';

                lastUpdate.textContent = 'Sensor not responding';
            }
        }

        function refreshData() {
            const loading = document.getElementById('loading');
            loading.style.display = 'block';

            fetch('/api/sensor')
                .then(response => response.json())
                .then(data => {
                    loading.style.display = 'none';
                    updateData(data);
                })
                .catch(error => {
                    loading.style.display = 'none';
                    console.error('Error fetching data:', error);
                    const status = document.getElementById('status');
                    status.className = 'status offline';
                    status.textContent = '❌ Connection Error';
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
