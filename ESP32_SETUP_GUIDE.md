# ESP32-S3 Smartwatch Setup Guide

Complete guide to program your Waveshare ESP32-S3-Touch-LCD-1.69 with BLE firmware.

## 📋 Prerequisites

### Hardware
- ✅ Waveshare ESP32-S3-Touch-LCD-1.69 module
- ✅ USB-C cable for programming
- ✅ Computer (Windows/Mac/Linux)

### Software
- [ ] Arduino IDE 2.x or Arduino IDE 1.8.x
- [ ] ESP32 Board Support Package
- [ ] USB drivers (usually automatic)

---

## 🛠️ Step 1: Install Arduino IDE

### Option A: Arduino IDE 2.x (Recommended)
1. Download from: https://www.arduino.cc/en/software
2. Install for your operating system
3. Launch Arduino IDE

### Option B: Arduino IDE 1.8.x
1. Download from: https://www.arduino.cc/en/software
2. Install and launch

---

## 📦 Step 2: Install ESP32 Board Support

### In Arduino IDE 2.x:

1. **Open Board Manager**
   - Click on the second icon on left sidebar (Board Manager)
   - Or go to: `Tools` → `Board` → `Boards Manager...`

2. **Search for ESP32**
   - Type "esp32" in search box
   - Find "esp32 by Espressif Systems"
   - Click **Install** (latest version)
   - Wait for installation to complete

3. **Verify Installation**
   - Go to `Tools` → `Board`
   - You should see "ESP32 Arduino" section with many boards

### Manual Installation (if above fails):

1. **Add Board Manager URL**
   - Go to `File` → `Preferences`
   - Find "Additional Board Manager URLs"
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Click OK

2. **Install from Board Manager** (repeat steps above)

---

## 🔌 Step 3: Connect Your ESP32-S3

1. **Connect USB Cable**
   - Plug USB-C cable into ESP32-S3
   - Connect other end to computer
   - Watch should power on

2. **Install Drivers (if needed)**
   - **Windows**: Usually automatic
   - **Mac**: Usually automatic
   - **Linux**: May need to add user to dialout group:
     ```bash
     sudo usermod -a -G dialout $USER
     # Then log out and back in
     ```

3. **Check COM Port**
   - Windows: Open Device Manager → Ports (COM & LPT)
   - Mac/Linux: Open Terminal, run `ls /dev/tty.*`
   - Note the port name (e.g., COM3, /dev/ttyUSB0)

---

## ⚙️ Step 4: Configure Arduino IDE

1. **Select Board**
   - Go to `Tools` → `Board` → `ESP32 Arduino`
   - Select: **"ESP32S3 Dev Module"**

2. **Configure Board Settings**
   ```
   Board:              "ESP32S3 Dev Module"
   USB CDC On Boot:    "Enabled"
   CPU Frequency:      "240MHz (WiFi)"
   Flash Mode:         "QIO 80MHz"
   Flash Size:         "16MB (128Mb)"
   Partition Scheme:   "16M Flash (3MB APP/9.9MB FATFS)"
   PSRAM:              "OPI PSRAM"
   Upload Mode:        "UART0 / Hardware CDC"
   Upload Speed:       "921600"
   USB Mode:           "Hardware CDC and JTAG"
   ```

3. **Select Port**
   - Go to `Tools` → `Port`
   - Select the COM port from Step 3

---

## 📝 Step 5: Open and Upload Firmware

1. **Open the Firmware**
   - Go to `File` → `Open`
   - Navigate to: `C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\esp32-c3-mini\`
   - Select: `ESP32_S3_HealthWatch_BLE.ino`
   - Click **Open**

2. **Verify the Code**
   - Click the checkmark icon (✓) or `Ctrl+R`
   - Wait for compilation to complete
   - Should say "Done compiling"

3. **Upload to ESP32**
   - Click the arrow icon (→) or `Ctrl+U`
   - Wait for upload to start
   - You may need to press **BOOT** button on ESP32 if it doesn't auto-enter upload mode
   - Wait for "Hard resetting via RTS pin..." message
   - Should say "Done uploading"

---

## 🔍 Step 6: Test the Firmware

1. **Open Serial Monitor**
   - Click magnifying glass icon (🔍)
   - Or go to `Tools` → `Serial Monitor`
   - Set baud rate to **115200**

2. **Press RESET Button on ESP32**

3. **Expected Output**
   ```
   ╔════════════════════════════════════════╗
   ║   ESP32-S3 Health Watch - BLE Mode    ║
   ║   Waveshare Touch LCD 1.69            ║
   ╚════════════════════════════════════════╝

   🔵 Initializing BLE...
   ✅ BLE initialized and advertising
   📱 Device Name: Chronos C3
   🔍 Waiting for connection...
   ✅ Setup complete!
   ```

4. **Verify BLE is Working**
   - Use a BLE scanner app on your phone:
     - Android: "nRF Connect"
     - iOS: "LightBlue Explorer"
   - Scan for devices
   - You should see "Chronos C3"

---

## 🎨 Step 7: Customize (Optional)

### Change Device Name

In `ESP32_S3_HealthWatch_BLE.ino`, line ~31:
```cpp
#define DEVICE_NAME "Chronos C3"  // Change this to your preferred name
```

Change to something like:
```cpp
#define DEVICE_NAME "MyWatch"
```

### Adjust Data Send Interval

Line ~42:
```cpp
#define DATA_SEND_INTERVAL 5000  // Send data every 5 seconds
```

Change to send more/less frequently (in milliseconds).

### Modify LED Pin

Line ~45:
```cpp
#define LED_PIN 2  // Built-in LED
```

Adjust based on your board's LED pin.

---

## 🔧 Troubleshooting

### "Failed to connect to ESP32"
**Solutions:**
1. Try pressing and holding **BOOT** button while clicking upload
2. Release BOOT button after upload starts
3. Try lower upload speed (460800 or 115200)
4. Check USB cable (must support data, not just charging)
5. Try different USB port

### "Sketch too big"
**Solution:**
- Change Partition Scheme to "Huge APP (3MB No OTA/1MB SPIFFS)"

### "Port not found"
**Solutions:**
1. Reinstall USB drivers
2. Try different USB cable
3. Check Device Manager (Windows) or `dmesg` (Linux)
4. Restart computer

### "BLE not advertising"
**Solutions:**
1. Check Serial Monitor for errors
2. Press RESET button on ESP32
3. Power cycle the device
4. Re-upload firmware

### "No data in Serial Monitor"
**Solutions:**
1. Check baud rate is **115200**
2. Press RESET button
3. Try unplugging and replugging USB

---

## 📱 Next Steps: Connect to Mobile App

Once firmware is working:

1. **Build Expo Development Build**
   ```bash
   cd C:\Users\Vuyo\Downloads\Prototype2 (2)\Prototype2\portfolio-proff\health-tracking-app
   npx expo prebuild
   npx expo run:android
   ```

2. **Connect Watch to App**
   - Open Health Tracking app
   - Go to Settings
   - Tap "Connect Watch"
   - Select "Chronos C3"
   - Wait for connection

3. **Verify Data Flow**
   - Check Serial Monitor on ESP32
   - Should see: "📱 Device connected!"
   - Should see periodic data sends
   - Check app for steps, heart rate, battery

---

## 📚 Additional Resources

### ESP32-S3 Documentation
- Espressif Docs: https://docs.espressif.com/projects/arduino-esp32/
- Arduino ESP32 GitHub: https://github.com/espressif/arduino-esp32

### Waveshare Documentation
- Product Page: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69
- Demos and Examples: Check Waveshare's official GitHub

### BLE Resources
- Nordic UART Service: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/bluetooth_services/services/nus.html
- ESP32 BLE Arduino: https://github.com/nkolban/ESP32_BLE_Arduino

---

## 🆘 Getting Help

If you encounter issues:

1. Check Serial Monitor output
2. Use BLE scanner app to verify advertising
3. Review ESP32_BLE_INTEGRATION.md in the health-tracking-app folder
4. Check connections and power

---

## ✅ Success Checklist

- [ ] Arduino IDE installed
- [ ] ESP32 board support installed
- [ ] ESP32-S3 connected via USB
- [ ] Correct board selected in Tools menu
- [ ] COM port selected
- [ ] Firmware compiled without errors
- [ ] Firmware uploaded successfully
- [ ] Serial Monitor shows initialization
- [ ] BLE advertising visible in scanner app
- [ ] Ready to connect mobile app!

---

**Last Updated**: November 9, 2025
**Firmware Version**: 1.0.0
