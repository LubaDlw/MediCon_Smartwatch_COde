/**
 * @file wifi_server.cpp
 * @brief WiFi HTTP Server implementation for ESP32-S3 Smartwatch
 * Provides REST API endpoints for mobile app communication
 */

#include "wifi_server.h"

#ifdef ENABLE_WIFI

#include <ChronosESP32.h>
#include <Timber.h>
#include <Preferences.h>
#include <lvgl.h>
#include "ui/ui.h"
#include "apps/steps/steps.h"

// Global WiFi server instance
WatchWiFiServer wifiServer;

// External references from app_hal
extern ChronosESP32 watch;
extern Preferences prefs;

/**
 * Constructor - Initialize server on port 80
 */
WatchWiFiServer::WatchWiFiServer() : server(80), ap_mode(false), last_connection_attempt(0) {
    saved_ssid = "";
    saved_password = "";
}

/**
 * Initialize WiFi and HTTP server
 */
void WatchWiFiServer::begin() {
    Timber.i("WiFi Server: Initializing...");
    
    // Load saved WiFi credentials from preferences
    loadWiFiCredentials();
    
    // Try to connect to WiFi
    bool connected = connectToWiFi();
    
    if (!connected) {
        Timber.w("WiFi: Failed to connect, starting AP mode");
        startAPMode();
    }
    
    // Setup HTTP routes
    setupRoutes();
    
    // Start the async web server
    server.begin();
    
    Timber.i("WiFi Server: Started successfully");
    
    // Display connection status on watch screen
    displayConnectionStatus();
}

/**
 * Connect to WiFi network
 */
bool WatchWiFiServer::connectToWiFi(const char* ssid, const char* password) {
    // Use provided credentials or fall back to saved/default
    String connect_ssid = (ssid != nullptr) ? String(ssid) : 
                          (!saved_ssid.isEmpty() ? saved_ssid : String(DEFAULT_WIFI_SSID));
    String connect_password = (password != nullptr) ? String(password) : 
                              (!saved_password.isEmpty() ? saved_password : String(DEFAULT_WIFI_PASSWORD));
    
    Timber.i("WiFi: Connecting to '%s'...", connect_ssid.c_str());
    
    // Disconnect if already connected
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(100);
    }
    
    // Set WiFi mode to station
    WiFi.mode(WIFI_STA);
    WiFi.begin(connect_ssid.c_str(), connect_password.c_str());
    
    // Wait for connection with timeout
    uint32_t start_time = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < WIFI_CONNECT_TIMEOUT) {
        delay(500);
        Timber.d("WiFi: Connecting...");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        ap_mode = false;
        last_connection_attempt = millis();
        
        Timber.i("WiFi: Connected successfully!");
        Timber.i("WiFi: IP Address: %s", WiFi.localIP().toString().c_str());
        Timber.i("WiFi: SSID: %s", WiFi.SSID().c_str());
        
        // Save credentials if they were provided
        if (ssid != nullptr && password != nullptr) {
            saveWiFiCredentials(ssid, password);
        }
        
        return true;
    }
    
    Timber.w("WiFi: Connection failed (timeout)");
    return false;
}

/**
 * Start Access Point mode for configuration
 */
void WatchWiFiServer::startAPMode() {
    Timber.i("WiFi: Starting AP mode...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress ip = WiFi.softAPIP();
    ap_mode = true;
    
    Timber.i("WiFi: AP started successfully");
    Timber.i("WiFi: SSID: %s", AP_SSID);
    Timber.i("WiFi: Password: %s", AP_PASSWORD);
    Timber.i("WiFi: IP Address: %s", ip.toString().c_str());
}

/**
 * Check if connected to WiFi
 */
bool WatchWiFiServer::isConnected() {
    return WiFi.status() == WL_CONNECTED && !ap_mode;
}

/**
 * Get current IP address
 */
String WatchWiFiServer::getIPAddress() {
    if (ap_mode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

/**
 * Get current SSID
 */
String WatchWiFiServer::getSSID() {
    if (ap_mode) {
        return String(AP_SSID);
    }
    return WiFi.SSID();
}

/**
 * Save WiFi credentials to preferences
 */
void WatchWiFiServer::saveWiFiCredentials(const char* ssid, const char* password) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
    
    saved_ssid = String(ssid);
    saved_password = String(password);
    
    Timber.i("WiFi: Credentials saved");
}

/**
 * Load WiFi credentials from preferences
 */
void WatchWiFiServer::loadWiFiCredentials() {
    prefs.begin("wifi", true);
    saved_ssid = prefs.getString("ssid", "");
    saved_password = prefs.getString("password", "");
    prefs.end();
    
    if (!saved_ssid.isEmpty()) {
        Timber.i("WiFi: Loaded saved credentials for '%s'", saved_ssid.c_str());
    } else {
        Timber.i("WiFi: No saved credentials, using defaults");
    }
}

/**
 * Update WiFi connection status (called from main loop)
 */
void WatchWiFiServer::update() {
    // Check if we're in station mode but disconnected
    if (!ap_mode && WiFi.status() != WL_CONNECTED) {
        // Try to reconnect every 30 seconds
        if (millis() - last_connection_attempt > 30000) {
            Timber.w("WiFi: Connection lost, attempting to reconnect...");
            connectToWiFi();
        }
    }
}

/**
 * Display connection status on watch screen
 */
void WatchWiFiServer::displayConnectionStatus() {
    // This will be called to show the IP address on the watch display
    // You can add LVGL code here to create a popup or update a label
    String status = "WiFi: ";
    if (ap_mode) {
        status += "AP Mode\n" + getIPAddress() + "\nSSID: " + AP_SSID;
    } else if (isConnected()) {
        status += "Connected\n" + getIPAddress();
    } else {
        status += "Disconnected";
    }
    
    Timber.i(status.c_str());
    
    // Optional: Show on screen using LVGL
    // You can create a temporary label or popup here
}

/**
 * Setup all HTTP routes
 */
void WatchWiFiServer::setupRoutes() {
    Timber.i("WiFi Server: Setting up routes...");
    
    // Enable CORS for all routes
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Handle OPTIONS preflight requests
    server.on("/api/steps", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    server.on("/api/battery", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    server.on("/api/status", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    server.on("/api/time", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    server.on("/api/wifi/save", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    
    // GET /api/steps - Get step count
    server.on("/api/steps", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetSteps(request);
    });
    
    // GET /api/heartrate - Get heart rate
    server.on("/api/heartrate", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetHeartRate(request);
    });
    
    // GET /api/battery - Get battery level
    server.on("/api/battery", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetBattery(request);
    });
    
    // GET /api/status - Get watch status
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetStatus(request);
    });
    
    // POST /api/time - Sync time from phone
    server.on("/api/time", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // Handle body parsing in body handler
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Body handler
        this->handleSyncTime(request);
    });
    
    // GET /config - Configuration page (HTML)
    server.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleConfigPage(request);
    });
    
    // POST /api/wifi/save - Save WiFi credentials
    server.on("/api/wifi/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // Handle body parsing in body handler
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        this->handleSaveWiFi(request);
    });
    
    // ============================================================
    // MOBILE APP COMPATIBILITY ROUTES (for Expo health tracking app)
    // These routes work alongside the existing /api/* endpoints
    // ============================================================
    
    // GET / - Root info page (mobile app compatible)
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        int steps = get_step_count();
        int heartRate = 75;  // Mock - replace with real sensor when available
        int battery = 85;    // Mock - replace with real battery reading
        
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<style>body{font-family:Arial;margin:20px;background:#f0f9ff;}";
        html += "h1{color:#1e40af;}.card{background:white;padding:20px;border-radius:10px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
        html += ".data{font-size:24px;font-weight:bold;color:#22c55e;}</style></head><body>";
        html += "<h1>🏥 Chronos C3 Health Watch</h1>";
        html += "<div class='card'><h2>📊 Current Data</h2>";
        html += "<p>Steps: <span class='data'>" + String(steps) + "</span></p>";
        html += "<p>Heart Rate: <span class='data'>" + String(heartRate) + " BPM</span></p>";
        html += "<p>Battery: <span class='data'>" + String(battery) + "%</span></p></div>";
        html += "<div class='card'><h2>📱 Mobile App Connection</h2>";
        html += "<p>Enter this IP in your mobile app:</p>";
        html += "<p><b>" + getIPAddress() + "</b></p></div>";
        html += "<div class='card'><h2>🔌 API Endpoints</h2>";
        html += "<p>GET /data - Get sensor data (JSON)</p>";
        html += "<p>POST /sync-time - Sync time</p>";
        html += "<p>POST /weather - Update weather</p>";
        html += "<p>GET /config - WiFi configuration</p></div>";
        html += "</body></html>";
        
        request->send(200, "text/html", html);
        Timber.d("API: GET / - Info page served");
    });
    
    // GET /data - Combined sensor data (mobile app uses this endpoint)
    server.on("/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        int steps = get_step_count();
        int heartRate = 75;  // Mock - replace with real heart rate sensor
        int battery = 85;    // Mock - replace with real battery monitoring
        
        String json = "{";
        json += "\"steps\":" + String(steps) + ",";
        json += "\"heartRate\":" + String(heartRate) + ",";
        json += "\"battery\":" + String(battery) + ",";
        json += "\"timestamp\":" + String(millis());
        json += "}";
        
        request->send(200, "application/json", json);
        Timber.d("API: GET /data - Steps:%d HR:%d Battery:%d", steps, heartRate, battery);
    });
    
    // OPTIONS /data - CORS preflight
    server.on("/data", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    
    // POST /sync-time - Sync time from mobile app
    server.on("/sync-time", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("timestamp", true)) {
            String timestamp = request->getParam("timestamp", true)->value();
            Timber.i("API: POST /sync-time - Timestamp: %s", timestamp.c_str());
            
            // TODO: Integrate with your watch time system
            // long unixTime = timestamp.toInt();
            // watch.setTime(...);
            
            String json = "{\"status\":\"ok\",\"message\":\"Time synced\"}";
            request->send(200, "application/json", json);
        } else {
            String json = "{\"status\":\"error\",\"message\":\"No timestamp provided\"}";
            request->send(400, "application/json", json);
        }
    });
    
    // OPTIONS /sync-time - CORS preflight
    server.on("/sync-time", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    
    // POST /weather - Receive weather data from mobile app
    server.on("/weather", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("temperature", true) && request->hasParam("condition", true)) {
            String temp = request->getParam("temperature", true)->value();
            String condition = request->getParam("condition", true)->value();
            
            Timber.i("API: POST /weather - Temp:%s°C Condition:%s", temp.c_str(), condition.c_str());
            
            // TODO: Integrate with your watch weather display
            // You can update the watch UI to show this weather data
            
            String json = "{\"status\":\"ok\",\"message\":\"Weather updated\"}";
            request->send(200, "application/json", json);
        } else {
            String json = "{\"status\":\"error\",\"message\":\"Missing parameters\"}";
            request->send(400, "application/json", json);
        }
    });
    
    // OPTIONS /weather - CORS preflight
    server.on("/weather", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
    
    Timber.i("WiFi Server: Routes configured (including mobile app compatibility)");
}

/**
 * Handle GET /api/steps
 */
void WatchWiFiServer::handleGetSteps(AsyncWebServerRequest *request) {
    int steps = get_step_count();
    float distance = get_distance_km();
    int calories = get_calories();
    
    String json = "{";
    json += "\"steps\":" + String(steps) + ",";
    json += "\"distance\":" + String(distance, 2) + ",";
    json += "\"calories\":" + String(calories);
    json += "}";
    
    request->send(200, "application/json", json);
    Timber.d("API: GET /api/steps - %d steps", steps);
}

/**
 * Handle GET /api/heartrate
 */
void WatchWiFiServer::handleGetHeartRate(AsyncWebServerRequest *request) {
    // Mock heart rate data (replace with real sensor data if available)
    int heartRate = 75;
    
    String json = "{\"heartRate\":" + String(heartRate) + "}";
    
    request->send(200, "application/json", json);
    Timber.d("API: GET /api/heartrate - %d bpm", heartRate);
}

/**
 * Handle GET /api/battery
 */
void WatchWiFiServer::handleGetBattery(AsyncWebServerRequest *request) {
    // Mock battery data (replace with real battery sensor if available)
    int battery = 85;
    
    String json = "{\"battery\":" + String(battery) + "}";
    
    request->send(200, "application/json", json);
    Timber.d("API: GET /api/battery - %d%%", battery);
}

/**
 * Handle GET /api/status
 */
void WatchWiFiServer::handleGetStatus(AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"connected\":" + String(isConnected() ? "true" : "false") + ",";
    json += "\"ip\":\"" + getIPAddress() + "\",";
    json += "\"ssid\":\"" + getSSID() + "\",";
    json += "\"apMode\":" + String(ap_mode ? "true" : "false") + ",";
    json += "\"battery\":" + String(85) + ",";
    json += "\"steps\":" + String(get_step_count());
    json += "}";
    
    request->send(200, "application/json", json);
    Timber.d("API: GET /api/status");
}

/**
 * Handle POST /api/time
 */
void WatchWiFiServer::handleSyncTime(AsyncWebServerRequest *request) {
    // Parse JSON body (simplified version)
    // In production, use ArduinoJson for proper parsing
    
    // For now, just acknowledge the request
    String json = "{\"success\":true,\"message\":\"Time sync received\"}";
    
    request->send(200, "application/json", json);
    Timber.i("API: POST /api/time - Time sync requested");
}

/**
 * Handle GET /config - Configuration page
 */
void WatchWiFiServer::handleConfigPage(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<title>ESP32 Watch Setup</title>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += ".container{max-width:400px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;}input{width:100%;padding:10px;margin:10px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box;}";
    html += "button{width:100%;padding:12px;background:#007bff;color:white;border:none;border-radius:5px;cursor:pointer;font-size:16px;}";
    html += "button:hover{background:#0056b3;}.info{background:#e7f3ff;padding:10px;border-radius:5px;margin:10px 0;}</style></head>";
    html += "<body><div class=\"container\"><h1>ESP32 Watch Setup</h1>";
    html += "<div class=\"info\">Current IP: " + getIPAddress() + "<br>SSID: " + getSSID() + "</div>";
    html += "<form action=\"/api/wifi/save\" method=\"POST\">";
    html += "<input type=\"text\" name=\"ssid\" placeholder=\"WiFi SSID\" required>";
    html += "<input type=\"password\" name=\"password\" placeholder=\"WiFi Password\" required>";
    html += "<button type=\"submit\">Save & Connect</button>";
    html += "</form></div></body></html>";
    
    request->send(200, "text/html", html);
    Timber.d("API: GET /config - Configuration page served");
}

/**
 * Handle POST /api/wifi/save
 */
void WatchWiFiServer::handleSaveWiFi(AsyncWebServerRequest *request) {
    String ssid = "";
    String password = "";
    
    // Parse form data
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }
    
    if (ssid.length() > 0 && password.length() > 0) {
        saveWiFiCredentials(ssid.c_str(), password.c_str());
        
        String json = "{\"success\":true,\"message\":\"Credentials saved. Reconnecting...\"}";
        request->send(200, "application/json", json);
        
        Timber.i("API: POST /api/wifi/save - New credentials saved");
        
        // Reconnect with new credentials
        delay(1000);
        connectToWiFi(ssid.c_str(), password.c_str());
    } else {
        String json = "{\"success\":false,\"message\":\"Invalid credentials\"}";
        request->send(400, "application/json", json);
    }
}

/**
 * Helper: Get JSON string for key-value pair (int)
 */
String WatchWiFiServer::getJSON(const char* key, int value) {
    return "{\"" + String(key) + "\":" + String(value) + "}";
}

/**
 * Helper: Get JSON string for key-value pair (string)
 */
String WatchWiFiServer::getJSON(const char* key, const char* value) {
    return "{\"" + String(key) + "\":\"" + String(value) + "\"}";
}

/**
 * Helper: Set CORS headers (already handled by DefaultHeaders)
 */
void WatchWiFiServer::setCORSHeaders(AsyncWebServerRequest *request) {
    // CORS headers are set globally via DefaultHeaders
}

// ============================================================
// C WRAPPER FUNCTIONS (for app_hal.cpp compatibility)
// ============================================================

/**
 * Initialize WiFi server (C wrapper function)
 * Called from app_hal.cpp setup
 */
void wifi_setup() {
    wifiServer.begin();
}

/**
 * Update WiFi server (C wrapper function)
 * Called from app_hal.cpp loop
 */
void wifi_loop() {
    wifiServer.update();
}

#endif // ENABLE_WIFI
