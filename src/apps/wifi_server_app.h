/**
 * WiFi Server App Header
 * For ChronosESP32 Health Watch
 */

#ifndef WIFI_SERVER_APP_H
#define WIFI_SERVER_APP_H

#ifdef ENABLE_APP_WIFI_SERVER

// Function declarations
void wifiServerSetup();
void wifiServerLoop();
void initWiFi();
void setupServer();
void handleRoot();
void handleData();
void handleTimeSync();
void handleWeather();
void updateSensorData();
void checkWiFiConnection();

#endif // ENABLE_APP_WIFI_SERVER

#endif // WIFI_SERVER_APP_H
