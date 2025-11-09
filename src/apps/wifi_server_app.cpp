/**
 * WiFi Server App for ChronosESP32
 * Sends health data to mobile app via HTTP
 * 
 * This app creates a web server that provides:
 * - Steps count from QMI8658C sensor
 * - Heart rate (simulated or from sensor)
 * - Battery level
 * 
 * Compatible with Health Tracking mobile app
 */

#include "wifi_server_app.h"
#include <WiFi.h>
#include <WebServer.h>
#include "app_hal.h"
#include "steps/steps.h"

#ifdef ENABLE_APP_WIFI_SERVER

// WiFi credentials - CHane when in diffrent venue or dif wifi network - future implementation
const char* WIFI_SSID = "SW-1009-6517_EXT";
const char* WIFI_PASSWORD = "4e9ebf21a3";

// Create web server on port 80
WebServer server(80);

// WiFi status
bool wifiConnected = false;
bool serverRunning = false;
unsigned long lastConnectionAttempt = 0;
const unsigned long CONNECTION_RETRY_INTERVAL = 30000; // 30 seconds

// Sensor data cache
int cachedSteps = 0;
int cachedHeartRate = 72;
int cachedBattery = 100;

/**
 * Initialize WiFi connection
 */
void initWiFi() {
    Serial.println("\n========================================");
    Serial.println("WiFi Server App - Starting");
    Serial.println("========================================");
    
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("ESP32-HealthWatch");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\n✅ WiFi Connected!");
        Serial.println("========================================");
        Serial.print("📡 IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("📶 Signal: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.println("========================================");
        Serial.println("\n⚠️  ENTER THIS IP IN YOUR MOBILE APP!");
        Serial.print("   IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();
    } else {
        wifiConnected = false;
        Serial.println("\n❌ WiFi Connection Failed");
        Serial.println("Will retry in 30 seconds...");
    }
    
    lastConnectionAttempt = millis();
}

/**
 * Setup web server routes
 */
void setupServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/sync-time", HTTP_POST, handleTimeSync);
    server.on("/weather", HTTP_POST, handleWeather);
    
    server.enableCORS(true);
    server.begin();
    
    serverRunning = true;
    Serial.println("✅ Web server started on port 80");
    Serial.println("📱 Ready for mobile app connection\n");
}

/**
 * Root endpoint - Info page
 */
void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='5'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }";
    html += "h1 { color: white; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }";
    html += ".card { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); padding: 20px; border-radius: 15px; margin: 10px 0; box-shadow: 0 8px 32px 0 rgba(31,38,135,0.37); }";
    html += ".metric { font-size: 48px; font-weight: bold; color: #fbbf24; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }";
    html += ".label { font-size: 18px; opacity: 0.9; }";
    html += ".endpoint { background: rgba(255,255,255,0.15); padding: 10px; border-radius: 8px; margin: 5px 0; }";
    html += "</style>";
    html += "</head><body>";
    
    html += "<h1>🏥 ESP32 Health Watch</h1>";
    
    html += "<div class='card'>";
    html += "<h2>📊 Live Data</h2>";
    html += "<div style='text-align: center;'>";
    html += "<div style='margin: 20px;'>";
    html += "<div class='label'>👣 Steps</div>";
    html += "<div class='metric'>" + String(cachedSteps) + "</div>";
    html += "</div>";
    html += "<div style='margin: 20px;'>";
    html += "<div class='label'>💓 Heart Rate</div>";
    html += "<div class='metric'>" + String(cachedHeartRate) + " BPM</div>";
    html += "</div>";
    html += "<div style='margin: 20px;'>";
    html += "<div class='label'>🔋 Battery</div>";
    html += "<div class='metric'>" + String(cachedBattery) + "%</div>";
    html += "</div>";
    html += "</div>";
    html += "<p style='font-size: 12px; opacity: 0.7;'>Auto-refresh every 5 seconds</p>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h2>🔌 API Endpoints</h2>";
    html += "<div class='endpoint'><b>GET /</b> - This info page</div>";
    html += "<div class='endpoint'><b>GET /data</b> - Get sensor data (JSON)</div>";
    html += "<div class='endpoint'><b>POST /sync-time</b> - Sync time</div>";
    html += "<div class='endpoint'><b>POST /weather</b> - Receive weather</div>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h2>📱 Mobile App Setup</h2>";
    html += "<p>1. Open Health Tracking App</p>";
    html += "<p>2. Go to Settings</p>";
    html += "<p>3. Enter IP: <b>" + WiFi.localIP().toString() + "</b></p>";
    html += "<p>4. Tap 'Connect Watch'</p>";
    html += "</div>";
    
    html += "</body></html>";
    
    server.send(200, "text/html", html);
    Serial.println("📄 Served info page");
}

/**
 * Data endpoint - Returns JSON
 */
void handleData() {
    // Update cached data from sensors
    updateSensorData();
    
    // Create JSON response
    String json = "{";
    json += "\"steps\":" + String(cachedSteps) + ",";
    json += "\"heartRate\":" + String(cachedHeartRate) + ",";
    json += "\"battery\":" + String(cachedBattery) + ",";
    json += "\"timestamp\":" + String(millis()) + ",";
    json += "\"connected\":true";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "application/json", json);
    
    Serial.printf("📤 Data sent: Steps=%d HR=%d Battery=%d%%\n", 
                  cachedSteps, cachedHeartRate, cachedBattery);
}

/**
 * Time sync endpoint
 */
void handleTimeSync() {
    if (server.hasArg("timestamp")) {
        String timestamp = server.arg("timestamp");
        Serial.println("🕐 Time sync: " + timestamp);
        
        // You can sync ChronosESP32 time here
        // chronos.setUnixTime(timestamp.toInt());
        
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"error\"}");
    }
}

/**
 * Weather endpoint
 */
void handleWeather() {
    if (server.hasArg("temperature") && server.hasArg("condition")) {
        String temp = server.arg("temperature");
        String condition = server.arg("condition");
        
        Serial.println("🌤️  Weather: " + temp + "°C, " + condition);
        
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"error\"}");
    }
}

/**
 * Update sensor data from ChronosESP32
 */
void updateSensorData() {
    // Get steps from ChronosESP32 step counter
    #ifdef ENABLE_APP_STEPS
    extern int getStepCount(); // Assuming there's a function to get steps
    // cachedSteps = getStepCount();
    #endif
    
    // Get real sensor data
    cachedSteps = get_step_count();
    cachedHeartRate = 72 + random(-5, 15); // Simulated heart rate with variation
    cachedBattery = 100; // For now, assume full battery (can be enhanced later)
    
    #ifdef ENABLE_APP_STEPS
    // Steps are already retrieved above via get_step_count()
    #endif
}

/**
 * Check WiFi connection and reconnect if needed
 */
void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED && !wifiConnected) {
        if (millis() - lastConnectionAttempt > CONNECTION_RETRY_INTERVAL) {
            Serial.println("Attempting WiFi reconnection...");
            initWiFi();
        }
    } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
        wifiConnected = true;
        if (!serverRunning) {
            setupServer();
        }
    } else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
        wifiConnected = false;
        serverRunning = false;
        Serial.println("❌ WiFi connection lost");
    }
}

/**
 * WiFi Server App Setup
 */
void wifiServerSetup() {
    Serial.println("Initializing WiFi Server App...");
    initWiFi();
    
    if (wifiConnected) {
        setupServer();
    }
}

/**
 * WiFi Server App Loop
 */
void wifiServerLoop() {
    checkWiFiConnection();
    
    if (serverRunning) {
        server.handleClient();
    }
}

#endif // ENABLE_APP_WIFI_SERVER
