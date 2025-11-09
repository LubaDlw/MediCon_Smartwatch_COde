# Quick Start Guide - ESP32 WiFi Health Watch

## 🚀 Quick Setup (5 Minutes)

### 1. Update WiFi Credentials
Edit `src/apps/wifi_server_app.cpp`:
```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";      // ← Change this
const char* WIFI_PASSWORD = "YOUR_PASSWORD";    // ← Change this
```

### 2. Upload to ESP32
- Connect ESP32 via USB
- Click **PlatformIO: Upload** in VS Code
- Wait for upload to complete

### 3. Get IP Address
- Open Serial Monitor
- Look for: `📡 IP Address: 192.168.1.XXX`
- **Write it down!**

### 4. Configure Mobile App
- Open health tracking app
- Go to Settings
- Enter the IP address
- Tap "Connect Watch"

### 5. Test Connection
Open browser and visit: `http://192.168.1.XXX/`
You should see live data dashboard!

---

## 📱 Data Being Sent

| Metric | Source | Format |
|--------|--------|--------|
| Steps | Real sensor (QMI8658C) | Integer |
| Heart Rate | Simulated (72±15 BPM) | Integer |
| Battery | Fixed at 100% | Integer (0-100) |

---

## 🔧 Troubleshooting

**WiFi won't connect?**
- Check SSID/password spelling
- Router must be 2.4GHz (not 5GHz)
- Try moving ESP32 closer to router

**App can't connect?**
- Both devices on same WiFi?
- IP address entered correctly?
- Try ping: `ping 192.168.1.XXX`

**No step data?**
- Walk around with the watch
- Sensor needs movement to detect steps

---

## 📊 API Endpoints

```
GET  /          → HTML dashboard
GET  /data      → JSON sensor data
POST /sync-time → Sync time from phone
POST /weather   → Send weather to watch
```

---

## 💡 Pro Tips

1. **Bookmark the dashboard** - `http://192.168.1.XXX/`
2. **Check serial monitor** for detailed logs
3. **Walk around** to see steps increase
4. **Same WiFi network** is required for both devices

---

For complete documentation, see: `WIFI_SMARTWATCH_SETUP_GUIDE.md`
