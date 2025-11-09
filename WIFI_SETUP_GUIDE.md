# WiFi Setup Guide for ESP32-S3 Health Watch

Complete guide to add WiFi connectivity to your Waveshare ESP32-S3 ChronosESP32 smartwatch.

---

## 📋 What You Have

Your ESP32-S3 project is using the **ChronosESP32** framework - a complete smartwatch OS with:
- ✅ Watch faces and UI
- ✅ Apps and games  
- ✅ QMI8658C IMU sensor support
- ✅ Step counting
- ✅ Battery monitoring

**Now adding:** WiFi web server to send health data to your mobile app!

---

## 🎯 What We've Added

### New Files Created:
1. **`src/apps/wifi_server_app.cpp`** - WiFi server implementation
2. **`src/apps/wifi_server_app.h`** - Header file

### Features:
- ✅ WiFi web server on port 80
- ✅ Sends steps, heart rate, battery as JSON
- ✅ Beautiful web dashboard (auto-refresh)
- ✅ Auto-reconnect if WiFi drops
- ✅ CORS enabled for mobile app
- ✅ Works with existing ChronosESP32 features

---

## 🚀 Setup Instructions

### Step 1: Install PlatformIO

**Option A: VS Code Extension (Recommended)**
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search "PlatformIO IDE"
4. Click Install
5. Restart VS Code

**Option B: Command Line**
```bash
pip install platformio
```

### Step 2: Open Project in PlatformIO

1. **In VS Code:**
   - File → Open Folder
   - Navigate to: `C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\esp32-c3-mini`
   - Click "Select Folder"

2. **Wait for PlatformIO to initialize** (first time takes 2-3 minutes)
   - It will download ESP32 platform and libraries
   - You'll see progress in the bottom terminal

### Step 3: Enable WiFi Server App

Edit `platformio.ini` and find the section for your board:

```ini
[env:lolin_s3_mini_1_69]
extends = esp32
board = esp32-s3-devkitc-1
# ... existing settings ...
build_flags = 
	${esp32.build_flags}
	-D ESPS3_1_69=1
	-D LV_MEM_SIZE=144U*1024U
	-D LV_USE_QRCODE=1
	-D ENABLE_APP_QMI8658C=1
	-D ENABLE_APP_STEPS=1
	-D ENABLE_APP_WIFI_SERVER=1    # ADD THIS LINE
```

**Just add this one line:**
```
-D ENABLE_APP_WIFI_SERVER=1
```

### Step 4: Update WiFi Credentials

Edit `src/apps/wifi_server_app.cpp` lines 18-19:

```cpp
// CHANGE THESE TO YOUR WIFI NETWORK
const char* WIFI_SSID = "SW-1009-6517_EXT";      // Your network name
const char* WIFI_PASSWORD = "4e9ebf21a3";         // Your password
```

**Your credentials are already set!** But you can change them if needed.

### Step 5: Integrate WiFi App into HAL

We need to call the WiFi server functions. Find where apps are initialized in your `hal/esp32/app_hal.cpp` or main setup/loop.

**Add to setup section:**
```cpp
#ifdef ENABLE_APP_WIFI_SERVER
#include "apps/wifi_server_app.h"
#endif

void hal_setup() {
    // ... existing setup code ...
    
    #ifdef ENABLE_APP_WIFI_SERVER
    wifiServerSetup();
    #endif
}
```

**Add to loop section:**
```cpp
void hal_loop() {
    // ... existing loop code ...
    
    #ifdef ENABLE_APP_WIFI_SERVER
    wifiServerLoop();
    #endif
}
```

### Step 6: Build and Upload

**Using PlatformIO in VS Code:**

1. **Connect your ESP32-S3 via USB**

2. **Build the project:**
   - Click PlatformIO icon on left sidebar
   - Under "lolin_s3_mini_1_69" → Click "Build"
   - Or press Ctrl+Alt+B

3. **Upload to device:**
   - Under "lolin_s3_mini_1_69" → Click "Upload"
   - Or press Ctrl+Alt+U

**Using Command Line:**
```bash
cd "C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\esp32-c3-mini"
pio run -e lolin_s3_mini_1_69 -t upload
```

### Step 7: Monitor Serial Output

1. **Open Serial Monitor:**
   - PlatformIO → lolin_s3_mini_1_69 → Monitor
   - Or press Ctrl+Alt+S

2. **Press RESET button on ESP32**

3. **Expected output:**
```
========================================
WiFi Server App - Starting
========================================
Connecting to: SW-1009-6517_EXT
..........
✅ WiFi Connected!
========================================
📡 IP Address: 192.168.1.XXX
📶 Signal: -45 dBm
========================================

⚠️  ENTER THIS IP IN YOUR MOBILE APP!
   IP: 192.168.1.XXX

✅ Web server started on port 80
📱 Ready for mobile app connection
```

4. **IMPORTANT: Note the IP address!** (e.g., 192.168.1.100)

---

## 🧪 Testing

### Test 1: Web Browser

1. **On your phone or computer, open browser**
2. Go to: `http://[ESP32-IP-ADDRESS]/`
3. **You should see a beautiful dashboard with:**
   - Live steps count
   - Heart rate
   - Battery level
   - Auto-refresh every 5 seconds

### Test 2: Test API Endpoint

Browser: `http://[ESP32-IP-ADDRESS]/data`

Expected JSON:
```json
{
  "steps": 1234,
  "heartRate": 72,
  "battery": 85,
  "timestamp": 123456,
  "connected": true
}
```

### Test 3: Mobile App Connection

1. **Run your Expo app:**
   ```bash
   cd "C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\health-tracking-app"
   npx expo start
   ```

2. **Open in Expo Go** (no build needed!)

3. **Go to Settings**
   - Enter the IP address from Serial Monitor
   - Tap "Connect Watch"
   - Data should start flowing!

---

## 📂 Project Structure

```
esp32-c3-mini/
├── platformio.ini              ← Add -D ENABLE_APP_WIFI_SERVER=1
├── src/
│   ├── main.cpp
│   ├── apps/
│   │   ├── wifi_server_app.cpp ← NEW: WiFi server code
│   │   └── wifi_server_app.h   ← NEW: Header file
│   └── hal/
│       └── esp32/
│           └── app_hal.cpp      ← Integrate WiFi calls here
```

---

## 🔧 Integration Steps (Detailed)

Since ChronosESP32 has its own structure, here's how to properly integrate:

### Option A: Standalone Mode (Simplest)

The WiFi server runs independently and doesn't interfere with watch functions.

**In `hal/esp32/app_hal.cpp` or `src/main.cpp`:**

```cpp
// Add at top of file
#ifdef ENABLE_APP_WIFI_SERVER
#include "apps/wifi_server_app.h"
#endif

// In setup function (after serial and other init):
void setup() {
    Serial.begin(115200);
    // ... existing ChronosESP32 setup ...
    
    #ifdef ENABLE_APP_WIFI_SERVER
    wifiServerSetup();  // Start WiFi server
    #endif
}

// In loop function:
void loop() {
    // ... existing ChronosESP32 loop ...
    
    #ifdef ENABLE_APP_WIFI_SERVER
    wifiServerLoop();  // Handle WiFi requests
    #endif
}
```

### Option B: As ChronosESP32 App

If you want WiFi server as a watch app with UI controls:

1. Create app icon and entry in apps menu
2. Add start/stop WiFi controls
3. Show connection status on watch display
4. Add WiFi settings page

(This is more advanced and optional)

---

## 🎨 Customization

### Change Update Frequency

In `wifi_server_app.cpp`, line 278:
```cpp
if (millis() - lastUpdate > 1000) {  // Update every 1 second
```

Change to:
```cpp
if (millis() - lastUpdate > 500) {  // Update every 0.5 seconds
```

### Add Real Sensor Data

Replace simulated data with real QMI8658C readings:

```cpp
void updateSensorData() {
    #ifdef ENABLE_APP_STEPS
    // Get actual steps from ChronosESP32
    cachedSteps = chronos.getStepCount();
    #endif
    
    // Get heart rate if you have sensor
    // cachedHeartRate = readHeartRateSensor();
    
    // Get actual battery
    cachedBattery = getBatteryPercentage();
}
```

### Customize Web Dashboard

Edit the HTML in `handleRoot()` function (line 105) to change:
- Colors
- Layout
- Metrics displayed
- Refresh rate

---

## 🔍 Troubleshooting

### "WiFi Connection Failed"

**Check:**
1. SSID and password are correct
2. ESP32 is in range of WiFi router
3. WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
4. No special characters in password causing issues

**Solution:**
- Double-check credentials in `wifi_server_app.cpp`
- Try moving ESP32 closer to router
- Check router settings

### "Build Error: undefined reference"

**Solution:**
Make sure you added the integration code to `app_hal.cpp`

### "Upload Failed"

**Solutions:**
1. Hold BOOT button → Click Upload → Release after "Connecting..."
2. Check COM port is correct
3. Try closing Serial Monitor before upload
4. Use lower upload speed in `platformio.ini`:
   ```ini
   upload_speed = 460800
   ```

### "Web page not loading"

**Check:**
1. Phone/computer on same WiFi network as ESP32
2. IP address is correct
3. ESP32 Serial Monitor shows "Web server started"
4. Try pinging the IP: `ping 192.168.1.XXX`

### "Mobile app can't connect"

**Solutions:**
1. Verify ESP32 web page works in browser first
2. Check IP address entered correctly in app
3. Both devices on same WiFi
4. Check app console logs for errors

---

## 📊 Data Flow

```
ESP32 Watch                          Mobile App
    │                                     │
    ├─ WiFi Connection              ┌─────┤
    │  (SW-1009-6517_EXT)          │     │
    │                               │     │
    ├─ Web Server (Port 80) ◄──────┘     │
    │                                     │
    ├─ /data endpoint                     │
    │  Returns JSON ──────────────────────►
    │  {steps, heartRate, battery}        │
    │                                     │
    └─ Sensors (QMI8658C, etc)           └─ Display Data
       └─ Step counting                      └─ Dashboard
       └─ Battery monitoring                 └─ Charts
```

---

## ✅ Success Checklist

**ESP32 Setup:**
- [ ] PlatformIO installed
- [ ] Project opened in VS Code
- [ ] WiFi credentials updated
- [ ] `ENABLE_APP_WIFI_SERVER=1` added to platformio.ini
- [ ] Integration code added to HAL
- [ ] Build successful
- [ ] Upload successful
- [ ] Serial Monitor shows IP address
- [ ] Web page loads in browser

**Mobile App:**
- [ ] Expo app running
- [ ] ESP32 IP entered in Settings
- [ ] "Connect Watch" successful
- [ ] Data updating in app
- [ ] Real-time sync working

---

## 🎉 Next Steps

Once WiFi is working:

### 1. **Integrate Real Sensors**
- Use actual QMI8658C step data
- Add heart rate sensor
- Read real battery voltage

### 2. **Add Features**
- Weather display on watch
- Notifications from phone
- Time synchronization
- Custom watch face with connectivity indicator

### 3. **Optimize**
- Add WiFi sleep mode
- Reduce update frequency
- Implement data caching

### 4. **Mobile App**
- Works in Expo Go (no build needed!)
- Add WiFi connection UI
- Show connection status
- Display real-time data

---

## 📞 Need Help?

### Check These First:
1. **Serial Monitor** - ESP32 shows all status messages
2. **Web Browser** - Test API directly
3. **WiFi Scanner** - Verify ESP32 is connected
4. **This Guide** - Most issues covered above

### Common Issues:
- **Can't build**: Make sure PlatformIO is fully installed
- **Can't upload**: Hold BOOT button on ESP32
- **No WiFi**: Check credentials and 2.4GHz network
- **No data**: Check sensors are enabled in platformio.ini

---

## 📚 Resources

- **ChronosESP32**: https://github.com/fbiego/ChronosESP32
- **PlatformIO**: https://platformio.org/
- **ESP32 Arduino**: https://docs.espressif.com/projects/arduino-esp32/
- **Your Mobile App**: `health-tracking-app` folder

---

**Version**: 1.0.0  
**Last Updated**: November 9, 2025  
**Hardware**: Waveshare ESP32-S3-Touch-LCD-1.69  
**Framework**: ChronosESP32 + WiFi Server  
**Works with**: Expo Go (no build needed!)
