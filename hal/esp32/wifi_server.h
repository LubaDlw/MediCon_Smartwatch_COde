/**
 * @file wifi_server.h
 * @brief WiFi HTTP Server for ESP32-S3 Smartwatch
 * Provides REST API endpoints for mobile app communication
 */

#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

// WiFi Configuration
#define DEFAULT_WIFI_SSID "SW-1009-6517_EXT"
#define DEFAULT_WIFI_PASSWORD "4e9ebf21a3"
#define WIFI_CONNECT_TIMEOUT 15000  // 15 seconds
#define AP_SSID "ESP32-Watch-Setup"
#define AP_PASSWORD "12345678"

// WiFi Server class (renamed to avoid conflict with Arduino WiFiServer)
class WatchWiFiServer {
public:
    WatchWiFiServer();
    
    // Initialize WiFi and HTTP server
    void begin();
    
    // WiFi connection methods
    bool connectToWiFi(const char* ssid = nullptr, const char* password = nullptr);
    void startAPMode();
    bool isConnected();
    String getIPAddress();
    String getSSID();
    
    // Preferences management
    void saveWiFiCredentials(const char* ssid, const char* password);
    void loadWiFiCredentials();
    
    // Update methods (called from main loop)
    void update();
    
    // Display status
    void displayConnectionStatus();

private:
    AsyncWebServer server;
    Preferences prefs;
    
    String saved_ssid;
    String saved_password;
    bool ap_mode;
    uint32_t last_connection_attempt;
    
    // Setup HTTP routes
    void setupRoutes();
    
    // API endpoint handlers
    void handleGetSteps(AsyncWebServerRequest *request);
    void handleGetHeartRate(AsyncWebServerRequest *request);
    void handleGetBattery(AsyncWebServerRequest *request);
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleSyncTime(AsyncWebServerRequest *request);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handleSetConfig(AsyncWebServerRequest *request);
    void handleConfigPage(AsyncWebServerRequest *request);
    void handleSaveWiFi(AsyncWebServerRequest *request);
    
    // Helper methods
    String getJSON(const char* key, int value);
    String getJSON(const char* key, const char* value);
    void setCORSHeaders(AsyncWebServerRequest *request);
};

// Global instance
extern WatchWiFiServer wifiServer;

#endif // WIFI_SERVER_H
