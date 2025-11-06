# MediCon Smartwatch Implementation Guide

## Project Overview
MediCon is a health and fitness-focused smartwatch application for ESP32-S3 devices, featuring real-time step counting, fall detection, calorie tracking, and a custom health-themed watch face.

## Implementation Summary

### 1. Steps Counter (✅ Completed)
**Location:** `src/apps/steps/`

**Features Implemented:**
- Real-time step counting using QMI8658C accelerometer
- Improved step detection algorithm with moving average filter
- Accurate step counting with threshold-based peak detection
- Automatic distance calculation (km)
- Real-time calorie calculation
- Reset functionality
- Progress arc showing steps towards 10,000 daily goal

**Key Parameters:**
- Step threshold: 1.15g
- Min step interval: 250ms
- Max step interval: 2000ms (for walk stop detection)
- Step length: 0.75m (average)
- Calories per step: 0.04 kcal

**Files Modified:**
- `src/apps/steps/steps.c` - Full implementation
- `src/apps/steps/steps.h` - Public API declarations
- `src/common/api.h` - IMU data structure and functions

### 2. Fall Detection (✅ Completed)
**Location:** `src/apps/steps/steps.c`

**Features Implemented:**
- Real-time fall detection using accelerometer
- High-impact detection (fall threshold: 2.5g)
- Freefall detection (threshold: 0.5g)
- Visual fall alert with auto-dismiss
- Cooldown period to prevent false positives (5 seconds)

**Algorithm:**
- Monitors acceleration magnitude continuously
- Detects sudden high acceleration (impact)
- Detects sudden low acceleration (freefall)
- Shows alert: "FALL DETECTED! Are you OK?"

### 3. Calories Calculation (✅ Completed)
**Formula:** `calories = steps × 0.04`

The calorie calculation is integrated into the step counter and updates in real-time with step count.

###  4. MediCon Main Watch Face (✅ Completed)
**Location:** `src/faces/medicon/`

**Design Features:**
- Health-focused green color scheme
- Large, readable time display (48pt font)
- Day and date indicator
- Prominent step counter with icon
- Calories burned display with flame icon
- Heart rate display with heart icon
- Medical green theme (#2ECC71)
- Dark green background for OLED efficiency

**Layout (based on wireframe):**
```
┌─────────────────────┐
│     THU 23          │  ← Day/Date
│                     │
│      10:08          │  ← Large Time
│                     │
│  🔥 316    ❤️ 82   │  ← Calories & Heart Rate
│                     │
│  👣 7,532           │  ← Steps (large)
└─────────────────────┘
```

### 5. Build Configuration (✅ Completed)
**File:** `platformio.ini`

**Enabled Features:**
```ini
-D ENABLE_APP_QMI8658C=1
-D ENABLE_APP_STEPS=1
```

These flags enable:
- QMI8658C IMU sensor support
- Steps counter application
- FastIMU library integration

### 6. Integration Points

#### Watch Face Integration
The MediCon watch face has been created and registered in:
- `src/common/generated_features.h` - Include statement added
- `src/faces/medicon/medicon.h` - Header file
- `src/faces/medicon/medicon.c` - Implementation

**To fully integrate the watch face:**
1. Register in `src/ui/ui.c` using `registerWatchface_cb()`
2. Call `face_medicon_init()` during UI initialization
3. Add to watchface selection list

#### Public API Functions
Located in `src/apps/steps/steps.h`:
```c
int get_step_count(void);      // Returns current step count
float get_distance_km(void);    // Returns distance in kilometers
int get_calories(void);         // Returns calories burned
```

These functions can be called from any watch face to display health metrics.

### 7. Health & Fitness Theme

**Color Scheme:**
- Primary: #2ECC71 (Medical Green)
- Accent: #27AE60 (Darker Green)
- Text: #DCF5EA (Light Mint)
- Background: #1A3329 (Dark Green)

**Icons:**
- Steps: Footprint/Shuffle symbol
- Calories: Flame/Charge symbol
- Heart Rate: Heart symbol
- General health-focused iconography

## Hardware Requirements

- **Device:** ESP32-S3 with 1.69" display (or compatible)
- **Sensor:** QMI8658C 6-axis IMU (accelerometer + gyroscope)
- **Display:** 240x280 or similar resolution
- **Libraries:** FastIMU, LovyanGFX, LVGL 9.3.0

## Building the Project

### Prerequisites
1. PlatformIO installed
2. ESP32 toolchain configured
3. Device connected via USB

### Build Steps
```bash
# Navigate to project directory
cd esp32-c3-mini

# Build for ESP32-S3 1.69" display
pio run -e lolin_s3_mini_1_69

# Upload to device
pio run -e lolin_s3_mini_1_69 --target upload

# Monitor serial output
pio device monitor
```

## Testing Procedures

### Steps Counter Testing
1. Open Steps app from app list
2. Walk normally - observe step count increasing
3. Stop walking - verify counting stops
4. Shake device rapidly - should not register as steps (anti-false-positive)
5. Check that distance and calories update proportionally

### Fall Detection Testing
⚠️ **Warning:** Test carefully to avoid device damage
1. While Steps app is open, simulate a sudden drop
2. Or create high acceleration by quick movement
3. Verify fall alert appears
4. Check cooldown period prevents repeated alerts

### Watch Face Testing
1. Navigate to watchface selection
2. Select MediCon watchface
3. Verify time updates every minute
4. Check that steps, calories, and heart rate display correctly
5. Verify color scheme matches health theme

## Code Architecture

### Steps Detection Flow
```
IMU Sensor (QMI8658C)
    ↓
get_imu_data() → Raw acceleration (ax, ay, az)
    ↓
calc_acc_magnitude() → Total acceleration magnitude
    ↓
apply_moving_average() → Smoothed magnitude
    ↓
detect_step_from_imu() → Step detection algorithm
    ↓
Update steps, distance, calories
    ↓
update_steps_display() → Update UI
```

### Fall Detection Flow
```
Acceleration magnitude
    ↓
detect_fall_from_imu()
    ├─→ High impact (>2.5g) → Fall detected
    ├─→ Freefall (<0.5g) → Fall detected
    └─→ Normal → Continue monitoring
    ↓
show_fall_alert() → Display alert panel
```

## Known Limitations

1. **Step Accuracy:** Accuracy depends on sensor calibration and walking style
2. **Fall Detection:** May have false positives with very rapid movements
3. **Calorie Calculation:** Uses simplified formula; not personalized
4. **Heart Rate:** Currently shows simulated data (82 BPM) - requires HR sensor integration

## Future Enhancements

1. **Heart Rate Monitoring:** Integrate actual HR sensor (e.g., MAX30102)
2. **Sleep Tracking:** Add sleep detection and analysis
3. **Activity Recognition:** Distinguish walking, running, cycling
4. **GPS Integration:** Accurate distance and route tracking
5. **Cloud Sync:** Save health data to cloud storage
6. **Goals & Achievements:** Gamification elements
7. **Personalization:** User weight, height for accurate calorie calculation

## Troubleshooting

### Steps Not Counting
- Check if QMI8658C sensor is detected: Run QMI8658C app
- Verify build flags include `-D ENABLE_APP_QMI8658C=1`
- Check I2C connection and sensor address (0x6B)
- Increase step threshold if too sensitive

### Fall Detection Too Sensitive
- Increase FALL_THRESHOLD value (currently 2.5g)
- Increase FALL_COOLDOWN_MS (currently 5000ms)

### Build Errors
- Ensure FastIMU library is installed
- Check that LVGL version is 9.3.0
- Verify platformio.ini configuration matches your hardware

## Contributors & License

This implementation builds upon the Chronos ESP32 framework by Felix Biego.

**Libraries Used:**
- LVGL 9.3.0 (MIT License)
- FastIMU (MIT License)
- ChronosESP32 by Felix Biego
- LovyanGFX by lovyan03

## Support

For issues related to:
- **Hardware:** Check sensor connections and power supply
- **Build:** Verify PlatformIO configuration
- **Functionality:** Review serial monitor output for debug messages

## Changelog

### Version 1.0.0 (Current)
- ✅ Implemented step counter with moving average filter
- ✅ Added fall detection with visual alerts
- ✅ Integrated calorie calculation
- ✅ Created MediCon health-themed watch face
- ✅ Enabled QMI8658C sensor support
- ✅ Added public API for health metrics

---

**Project Status:** Ready for testing and deployment
**Last Updated:** November 6, 2025
**Target Device:** ESP32-S3 with 1.69" display
