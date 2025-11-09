# WiFi Smart Watch Setup Guide
## ESP32-S3 to Expo Mobile App Communication

This guide explains how to set up WiFi communication between your ESP32-S3 smartwatch and the Expo mobile health tracking app.

---

## Overview

The system consists of two parts:
1. **ESP32-S3 Smartwatch**: Runs a web server that provides sensor data (steps, heart rate, battery)
2. **Expo Mobile App**: Fetches data from the ESP32 via HTTP requests over WiFi

---

## Part 1: ESP32-S3 Smartwatch Setup

### Step 1: Configure WiFi Credentials

Open `src/apps/wifi_server_app.cpp` and update your WiFi credentials:

```cpp
// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

**Important**: The ESP32 and your phone must be on the **same WiFi network**!

### Step 2: Enable WiFi Server App

The WiFi server app should already be enabled. To verify, check that `ENABLE_APP_WIFI_SERVER` is defined in your build configuration.

### Step 3: Upload to ESP32

1. Connect your ESP32-S3 to your computer via USB
2. In VS Code, click the **PlatformIO: Upload** button (or use the command palette)
3. Wait for the upload to complete

### Step 4: Find ESP32 IP Address

1. Open the **Serial Monitor** in VS Code (PlatformIO: Monitor)
2. Reset the ESP32 (press the reset button)
3. Look for output like this:

```
========================================
WiFi Server App - Starting
========================================
Connecting to: YOUR_WIFI_NAME
...........
✅ WiFi Connected!
========================================
📡 IP Address: 192.168.1.XXX
📶 Signal: -45 dBm
========================================

⚠️  ENTER THIS IP IN YOUR MOBILE APP!
   IP: 192.168.1.XXX
```

4. **Write down the IP address** (e.g., `192.168.1.100`)

---

## Part 2: Mobile App Setup

### Step 1: Navigate to App Directory

```bash
cd "C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\health-tracking-app"
```

### Step 2: Install Dependencies (if not already done)

```bash
npm install
```

### Step 3: Start the App

```bash
npm start
# or
npx expo start
```

### Step 4: Open App on Your Phone

1. Install **Expo Go** app from Play Store/App Store
2. Scan the QR code shown in your terminal
3. The app will open on your phone

### Step 5: Configure ESP32 IP in App

1. In the mobile app, go to **Settings**
2. Find the **ESP32 IP Address** field
3. Enter the IP address you noted from the serial monitor (e.g., `192.168.1.100`)
4. Tap **Connect Watch** or **Save**

---

## Part 3: Testing the Connection

### On the ESP32 Serial Monitor, you should see:

```
📤 Data sent: Steps=1234 HR=75 Battery=100%
📤 Data sent: Steps=1240 HR=76 Battery=100%
...
```

### On the Mobile App, you should see:

- Real-time step count updating
- Heart rate updating
- Battery level displayed
- Connection status showing "Connected"

### To Test in a Web Browser:

You can also test the ESP32 server by opening a browser on your computer (connected to the same WiFi) and visiting:

```
http://192.168.1.XXX/
```

Replace `XXX` with your ESP32's IP address. You should see a live dashboard with:
- Steps count
- Heart rate
- Battery level
- Auto-refreshing every 5 seconds

---

## Troubleshooting

### Problem: ESP32 won't connect to WiFi

**Solutions:**
- Double-check SSID and password in `wifi_server_app.cpp`
- Make sure your WiFi router is 2.4GHz (ESP32 doesn't support 5GHz)
- Try moving ESP32 closer to the router
- Check if your WiFi has MAC address filtering enabled

### Problem: Mobile app can't connect to ESP32

**Solutions:**
- Verify both devices are on the **same WiFi network**
- Check the IP address is entered correctly in the app
- Try pinging the ESP32 from your computer: `ping 192.168.1.XXX`
- Check if your router has AP isolation enabled (disable it)
- Temporarily disable any VPN on your phone

### Problem: Connection works but no data

**Solutions:**
- Check ESP32 serial monitor for errors
- Reset the ESP32 and try again
- Verify the step counter is initialized (walk a few steps)

### Problem: Data is all zeros

**Solutions:**
- The step counter needs movement to register steps
- Walk around with the watch for a few seconds
- Heart rate is simulated (72 ± random variation)
- Battery is currently set to 100% (can be enhanced later)

---

## API Endpoints

The ESP32 provides the following HTTP endpoints:

### GET /
- Returns HTML dashboard with live data
- Auto-refreshes every 5 seconds

### GET /data
- Returns JSON with sensor data
- Example response:
```json
{
  "steps": 1234,
  "heartRate": 75,
  "battery": 100,
  "timestamp": 123456,
  "connected": true
}
```

### POST /sync-time
- Syncs time from phone to watch
- Parameter: `timestamp` (Unix timestamp)

### POST /weather
- Sends weather data to watch
- Parameters: `temperature`, `condition`

---

## Data Format

The mobile app expects data in this format:

```javascript
{
  steps: <number>,      // Step count
  heartRate: <number>,  // BPM
  battery: <number>,    // Percentage (0-100)
  timestamp: <number>,  // Unix timestamp
  connected: <boolean>  // Connection status
}
```

The ESP32 provides exactly this format, so no modifications are needed!

---

## Customization

### Change Polling Interval

In the mobile app's `wifiService.js`, you can adjust how often it fetches data:

```javascript
startPolling(5000); // 5 seconds (default)
startPolling(2000); // 2 seconds (faster updates)
startPolling(10000); // 10 seconds (slower, saves battery)
```

### Add More Sensors

To add more health metrics:

1. In `wifi_server_app.cpp`, add to the `updateSensorData()` function
2. Update the JSON in `handleData()` to include the new data
3. The mobile app will automatically receive it

---

## Network Requirements

- **WiFi Type**: 2.4GHz (802.11 b/g/n)
- **Security**: WPA/WPA2 supported
- **Same Network**: ESP32 and phone must be on the same WiFi
- **No AP Isolation**: Router should allow device-to-device communication

---

## Power Consumption Tips

The WiFi server consumes more power than BLE. To optimize:

1. Reduce polling frequency in the mobile app
2. Use deep sleep when not actively syncing
3. Disable WiFi when not needed
4. Consider implementing on-demand WiFi activation

---

## Next Steps

1. Test the basic connection
2. Walk around to see step count increase
3. Try the web dashboard in your browser
4. Explore adding more sensors if needed

---

## Support

If you encounter issues:
1. Check the serial monitor for debug messages
2. Verify network settings
3. Test the `/data` endpoint in a browser first
4. Ensure both devices are on 2.4GHz WiFi

---

**Congratulations!** Your smartwatch should now be sending health data to your mobile app over WiFi! 🎉
