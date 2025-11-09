# WiFi Integration Guide for Your ESP32-C3 Watch

## Overview

This guide shows you exactly what to add to your existing ChronosESP32 watch to enable WiFi connectivity with your mobile app.

## What You'll Get

✅ WiFi web server running alongside existing BLE
✅ HTTP API endpoints for mobile app
✅ Real step count from existing step counter
✅ Heart rate data (when sensor added)
✅ Battery level
✅ All existing watch features remain unchanged

---

## Step 1: Add WiFi Libraries to platformio.ini

**File:** `platformio.ini`

Add these libraries to your `lib_deps`:

```ini
lib_deps =
    # ... your existing libraries ...
    ESP Async WebServer
    ESPAsyncTCP  ; for ESP8266
    AsyncTCP     ; for ESP32
```

---

## Step 2: Add WiFi Configuration

**File:** `hal/esp32/app_hal.h`

Add these lines after the existing includes:

```cpp
// Add WiFi configuration
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define ENABLE_WIFI_SERVER true

// WiFi functions
void wifi_setup();
void wifi_loop();
String wifi_get_ip();
```

---

## Step 3: Create WiFi Server Implementation

**Create New File:** `hal/esp32/wifi_server.cpp`

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include "app_hal.h"
#include "../../src/apps/steps/steps.h"

WebServer wifiServer(80);
bool wifiConnected = false;

// Mock heart rate (replace with real sensor when available)
int simulatedHeartRate = 72;

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; background: #f0f9ff; }";
    html += "h1 { color: #1e40af; }";
    html += ".card { background: white; padding: 20px; border-radius: 10px; margin: 10px 0; }";
    html += ".data { font-size: 24px; font-weight: bold; color: #22c55e; }";
    html += "</style></head><body>";
    
    html += "<h1>🏥 Chronos C3 Watch</h1>";
    html += "<div class='card'>";
    html += "<h2>📊 Current Data</h2>";
    html += "<p>Steps: <span class='data'>" + String(get_step_count()) + "</span></p>";
    html += "<p>Heart Rate: <span class='data'>" + String(simulatedHeartRate) + " BPM</span></p>";
    html += "<p>Battery: <span class='data'>85%</span></p>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h2>📱 Mobile App Connection</h2>";
    html += "<p>Enter IP in app: <b>" + WiFi.localIP().toString() + "</b></p>";
    html += "</div>";
    html += "</body></html>";
    
    wifiServer.send(200, "text/html", html);
    Serial.println("📄 Served info page");
}

void handleData() {
    // Get real step count from your existing step counter
    int steps = get_step_count();
    
    // Create JSON response
    String json = "{";
    json += "\"steps\":" + String(steps) + ",";
    json += "\"heartRate\":" + String(simulatedHeartRate) + ",";
    json += "\"battery\":85,";
    json += "\"timestamp\":" + String(millis());
    json += "}";
    
    // Send with CORS headers
    wifiServer.sendHeader("Access-Control-Allow-Origin", "*");
    wifiServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    wifiServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    wifiServer.send(200, "application/json", json);
    
    Serial.printf("📤 Data sent: Steps=%d HR=%d Battery=85%%\n", steps, simulatedHeartRate);
}

void handleTimeSync() {
    if (wifiServer.hasArg("timestamp")) {
        String timestamp = wifiServer.arg("timestamp");
        Serial.println("🕐 Time synced from app: " + timestamp);
        
        // You can integrate with your existing watch.setTime() here
        // long unixTime = timestamp.toInt();
        // Update RTC or watch time
        
        wifiServer.sendHeader("Access-Control-Allow-Origin", "*");
        wifiServer.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        wifiServer.send(400, "application/json", "{\"status\":\"error\"}");
    }
}

void handleWeather() {
    if (wifiServer.hasArg("temperature") && wifiServer.hasArg("condition")) {
        String temp = wifiServer.arg("temperature");
        String condition = wifiServer.arg("condition");
        
        Serial.println("🌤️  Weather from app: " + temp + "°C");
        
        // You can integrate with your existing weather display here
        
        wifiServer.sendHeader("Access-Control-Allow-Origin", "*");
        wifiServer.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        wifiServer.send(400, "application/json", "{\"status\":\"error\"}");
    }
}

void wifi_setup() {
    #ifndef ENABLE_WIFI_SERVER
    return;
    #endif
    
    Serial.println("\n========================================");
    Serial.println("WiFi Server Initialization");
    Serial.println("========================================");
    
    // Connect to WiFi
    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setHostname("Chronos-C3");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n❌ WiFi Connection Failed");
        wifiConnected = false;
        return;
    }
    
    wifiConnected = true;
    
    Serial.println("\n✅ WiFi Connected!");
    Serial.println("========================================");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("========================================");
    Serial.println("⚠️  Enter this IP in your mobile app!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    
    // Setup web server routes
    wifiServer.on("/", handleRoot);
    wifiServer.on("/data", handleData);
    wifiServer.on("/sync-time", HTTP_POST, handleTimeSync);
    wifiServer.on("/weather", HTTP_POST, handleWeather);
    
    // Enable CORS
    wifiServer.enableCORS(true);
    
    // Start server
    wifiServer.begin();
    Serial.println("✅ Web server started on port 80");
    Serial.println("📱 Ready for mobile app connection\n");
}

void wifi_loop() {
    #ifndef ENABLE_WIFI_SERVER
    return;
    #endif
    
    if (wifiConnected) {
        wifiServer.handleClient();
        
        // Simulate heart rate changes
        static unsigned long lastHR = 0;
        if (millis() - lastHR > 5000) {
            simulatedHeartRate = 60 + random(0, 40);
            lastHR = millis();
        }
    }
}

String wifi_get_ip() {
    if (wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}
```

---

## Step 4: Integrate into Main Code

**File:** `hal/esp32/app_hal.cpp`

### 4.1: Add at the top (after existing includes):

```cpp
// WiFi Server (at the top of the file, after other includes)
extern void wifi_setup();
extern void wifi_loop();
extern String wifi_get_ip();
```

### 4.2: In `hal_setup()` function, add AFTER `watch.begin()`:

```cpp
void hal_setup() {
    // ... all your existing setup code ...
    
    watch.begin();
    watch.set24Hour(true);
    watch.setBattery(85);
    
    // ADD THIS: Initialize WiFi server
    #ifdef ENABLE_WIFI_SERVER
    wifi_setup();
    #endif
    
    // ... rest of your existing setup code ...
}
```

### 4.3: In `hal_loop()` function, add at the END (before closing brace):

```cpp
void hal_loop() {
    // ... all your existing loop code ...
    
    // ADD THIS: Handle WiFi server requests
    #ifdef ENABLE_WIFI_SERVER
    wifi_loop();
    #endif
}
```

---

## Step 5: Update CMakeLists.txt (if using CMake)

If your project uses CMake, add the new WiFi file:

```cmake
set(SOURCES
    # ... existing sources ...
    hal/esp32/wifi_server.cpp
)
```

---

## Step 6: Configure Your WiFi

**File:** `hal/esp32/app_hal.h`

Change these lines to your actual WiFi credentials:

```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"      // Your WiFi network name
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"  // Your WiFi password
```

---

## Step 7: Build and Upload

1. **Open PlatformIO**
2. **Clean Build** (if you had previous builds)
3. **Build the project**
4. **Upload to ESP32-C3**
5. **Open Serial Monitor** (115200 baud)
6. **Note the IP address** displayed

---

## Step 8: Test the Connection

### Test 1: Open in Browser

```
http://YOUR_ESP32_IP/
```

You should see a nice dashboard with current steps, heart rate, and battery.

### Test 2: Test API Endpoint

```
http://YOUR_ESP32_IP/data
```

Should return JSON:
```json
{
  "steps": 1234,
  "heartRate": 72,
  "battery": 85,
  "timestamp": 123456
}
```

### Test 3: Connect Mobile App

1. Open your Health Tracking App (Expo Go)
2. Go to Settings
3. Find "ESP32 Smartwatch" section
4. Enter the IP address from Step 7
5. Tap "Connect Watch"
6. You should see real step data! 🎉

---

## Integration with Real Sensors

### Heart Rate Sensor (if you add one)

In `wifi_server.cpp`, replace:

```cpp
int simulatedHeartRate = 72;
```

With actual sensor reading:

```cpp
int getHeartRate() {
    // Add your heart rate sensor code here
    // Example: return heartRateSensor.getBPM();
    return 72; // placeholder
}
```

Then in `handleData()`:

```cpp
json += "\"heartRate\":" + String(getHeartRate()) + ",";
```

### Battery Level

Replace `85` with actual battery reading:

```cpp
int getBatteryLevel() {
    // Read from your battery monitoring circuit
    // Example: analogRead(BATTERY_PIN)
    return watch.getBattery(); // or your battery reading method
}
```

---

## Comparison: BLE vs WiFi

Your watch currently has:

| Feature | BLE (ChronosESP32) | WiFi (New) |
|---------|-------------------|------------|
| **Mobile App Support** | Android/iOS | Android/iOS |
| **Works in Expo Go** | ❌ No | ✅ Yes |
| **Range** | ~10 meters | ~50 meters |
| **Data Transfer** | Real-time | Polling (5s) |
| **Setup** | Already working | 5 minutes |
| **Battery Impact** | Low | Medium |

**Recommendation:** Keep both! Use BLE for notifications and WiFi for data sync.

---

## Troubleshooting

### WiFi Won't Connect

1. Check WiFi credentials in `app_hal.h`
2. Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
3. Check Serial Monitor for error messages
4. Try increasing timeout in `wifi_setup()`

### Can't Access Web Server

1. Ensure phone and ESP32 on same WiFi network
2. Check firewall settings
3. Try pinging the ESP32 IP
4. Verify web server started (check Serial Monitor)

### Mobile App Can't Connect

1. Verify IP address is correct
2. Test in browser first: `http://ESP32_IP/data`
3. Check CORS headers are being sent
4. Ensure both devices on same network

---

## Benefits of This Integration

✅ **Non-Invasive** - All existing features remain unchanged
✅ **Dual Connectivity** - Keep BLE and add WiFi
✅ **Real Data** - Uses your existing step counter
✅ **Easy to Extend** - Add more endpoints as needed
✅ **Works Immediately** - No custom app build required
✅ **Debuggable** - Test with browser before using app

---

## Next Steps

1. **Test WiFi connection** - Upload code and check Serial Monitor
2. **Test in browser** - Verify endpoints work
3. **Connect mobile app** - Enter IP and connect
4. **Add heart rate sensor** - When hardware is ready
5. **Customize data** - Add more health metrics

---

## Support

- ChronosESP32: https://github.com/fbiego/ChronosESP32
- ESP32 WiFi Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html

---

**Your watch will now work with the mobile app over WiFi, while keeping all existing BLE functionality intact!** 🎉

---

**Last Updated:** November 6, 2025
