/**
 * ESP32-S3 Health Watch - BLE Integration
 * For Waveshare ESP32-S3-Touch-LCD-1.69
 * 
 * This firmware implements BLE communication with the Health Tracking mobile app
 * using the Nordic UART Service (NUS) protocol.
 * 
 * Features:
 * - BLE advertising and connection
 * - Step counting (simulated or with IMU)
 * - Heart rate monitoring (simulated or with sensor)
 * - Battery level reporting
 * - Time synchronization
 * - Weather data reception
 * - Notifications
 * 
 * Hardware: Waveshare ESP32-S3-Touch-LCD-1.69
 * - ESP32-S3 microcontroller
 * - 1.69" touch LCD display
 * - QMI8658C IMU (for step counting)
 * 
 * Required Libraries:
 * - ESP32 BLE Arduino (built-in)
 * - Optional: Wire (for I2C IMU communication)
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define DEVICE_NAME "Chronos C3"  // Change this to customize your watch name

// Nordic UART Service UUIDs (ChronosESP32 compatible)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // App writes to watch
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Watch notifies app

// Packet type definitions (must match mobile app)
#define PACKET_STEPS       0x01
#define PACKET_HEART_RATE  0x02
#define PACKET_BATTERY     0x03
#define PACKET_TIME_SYNC   0x04
#define PACKET_WEATHER     0x05
#define PACKET_NOTIFICATION 0x06

// Data transmission interval (milliseconds)
#define DATA_SEND_INTERVAL 5000  // Send data every 5 seconds

// LED pin for status indication (adjust based on your board)
#define LED_PIN 2  // Built-in LED on most ESP32 boards

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

BLEServer* pServer = nullptr;
BLECharacteristic* pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Health data
uint16_t stepCount = 0;
uint16_t heartRate = 72;  // Normal resting heart rate
uint8_t batteryLevel = 100;
unsigned long lastDataSend = 0;
unsigned long lastStepIncrement = 0;

// Time synchronization
unsigned long syncedTime = 0;
unsigned long lastSyncMillis = 0;

// ============================================================================
// BLE CALLBACKS
// ============================================================================

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("📱 Device connected!");
      digitalWrite(LED_PIN, HIGH);  // Turn on LED when connected
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("📱 Device disconnected");
      digitalWrite(LED_PIN, LOW);  // Turn off LED when disconnected
      
      // Restart advertising for reconnection
      delay(500);
      pServer->startAdvertising();
      Serial.println("🔄 Restarting advertising...");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() >= 3) {
        uint8_t packetType = (uint8_t)rxValue[0];
        uint16_t value = (uint8_t)rxValue[1] | ((uint8_t)rxValue[2] << 8);
        
        handleCommand(packetType, value);
      }
    }
};

// ============================================================================
// COMMAND HANDLER
// ============================================================================

void handleCommand(uint8_t type, uint16_t value) {
  switch(type) {
    case PACKET_TIME_SYNC:
      syncedTime = value;
      lastSyncMillis = millis();
      Serial.printf("⏰ Time synced: %d\n", value);
      break;
      
    case PACKET_WEATHER: {
      uint8_t temperature = value & 0xFF;
      uint8_t condition = (value >> 8) & 0xFF;
      Serial.printf("🌤️  Weather: %d°C, Condition: %d\n", temperature, condition);
      // Display weather on LCD if available
      break;
    }
      
    case PACKET_NOTIFICATION:
      Serial.printf("🔔 Notification received: Type %d\n", value);
      // Show notification on watch display
      break;
      
    case PACKET_STEPS:
      // Request to send current step count
      sendSteps();
      break;
      
    case PACKET_HEART_RATE:
      // Request to send heart rate
      sendHeartRate();
      break;
      
    case PACKET_BATTERY:
      // Request to send battery level
      sendBattery();
      break;
      
    default:
      Serial.printf("❓ Unknown packet type: 0x%02X\n", type);
      break;
  }
}

// ============================================================================
// DATA TRANSMISSION FUNCTIONS
// ============================================================================

void sendPacket(uint8_t packetType, uint16_t value) {
  if (deviceConnected && pTxCharacteristic) {
    uint8_t data[3] = {
      packetType,
      (uint8_t)(value & 0xFF),
      (uint8_t)((value >> 8) & 0xFF)
    };
    
    pTxCharacteristic->setValue(data, 3);
    pTxCharacteristic->notify();
    
    delay(50);  // Small delay between transmissions
  }
}

void sendSteps() {
  sendPacket(PACKET_STEPS, stepCount);
  Serial.printf("📊 Sent steps: %d\n", stepCount);
}

void sendHeartRate() {
  sendPacket(PACKET_HEART_RATE, heartRate);
  Serial.printf("💓 Sent heart rate: %d BPM\n", heartRate);
}

void sendBattery() {
  sendPacket(PACKET_BATTERY, batteryLevel);
  Serial.printf("🔋 Sent battery: %d%%\n", batteryLevel);
}

void sendAllData() {
  if (deviceConnected) {
    sendSteps();
    delay(100);
    sendHeartRate();
    delay(100);
    sendBattery();
  }
}

// ============================================================================
// HEALTH DATA SIMULATION
// ============================================================================

void updateHealthData() {
  // Simulate step counting (increment steps periodically)
  if (millis() - lastStepIncrement > 2000) {  // Add steps every 2 seconds
    stepCount += random(5, 15);  // Random steps between 5-15
    if (stepCount > 10000) stepCount = 0;  // Reset at 10k steps for demo
    lastStepIncrement = millis();
  }
  
  // Simulate heart rate variation (between 60-100 BPM)
  static unsigned long lastHRUpdate = 0;
  if (millis() - lastHRUpdate > 10000) {  // Update every 10 seconds
    heartRate = random(60, 100);
    lastHRUpdate = millis();
  }
  
  // Simulate battery discharge (very slow)
  static unsigned long lastBatteryUpdate = 0;
  if (millis() - lastBatteryUpdate > 60000) {  // Update every minute
    if (batteryLevel > 0) batteryLevel--;
    if (batteryLevel < 20) batteryLevel = 100;  // Reset for demo
    lastBatteryUpdate = millis();
  }
}

// ============================================================================
// REAL SENSOR INTEGRATION (OPTIONAL)
// ============================================================================

// TODO: If you have QMI8658C IMU sensor, implement step counting here
/*
#include <Wire.h>

#define QMI8658C_ADDR 0x6B  // I2C address

void initIMU() {
  Wire.begin();
  // Initialize QMI8658C sensor
  // Configure accelerometer for step detection
}

uint16_t readStepsFromIMU() {
  // Read accelerometer data
  // Process for step detection
  // Return actual step count
  return stepCount;
}
*/

// TODO: If you have heart rate sensor, implement reading here
/*
uint16_t readHeartRate() {
  // Read from heart rate sensor
  // Process signal
  // Return BPM
  return heartRate;
}
*/

// TODO: Implement actual battery reading
/*
uint8_t readBatteryLevel() {
  // Read battery voltage via ADC
  // Convert to percentage
  return batteryLevel;
}
*/

// ============================================================================
// BLE INITIALIZATION
// ============================================================================

void initBLE() {
  Serial.println("🔵 Initializing BLE...");
  
  // Create BLE Device
  BLEDevice::init(DEVICE_NAME);
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create TX Characteristic (for notifications to app)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Create RX Characteristic (for writes from app)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // Functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("✅ BLE initialized and advertising");
  Serial.printf("📱 Device Name: %s\n", DEVICE_NAME);
  Serial.println("🔍 Waiting for connection...");
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   ESP32-S3 Health Watch - BLE Mode    ║");
  Serial.println("║   Waveshare Touch LCD 1.69            ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize BLE
  initBLE();
  
  // TODO: Initialize LCD display if available
  // TODO: Initialize IMU sensor if available
  // TODO: Initialize heart rate sensor if available
  
  Serial.println("✅ Setup complete!\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Handle connection state changes
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("🎉 New device connected!");
  }
  
  if (!deviceConnected && oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("👋 Device disconnected");
  }
  
  // Send data periodically when connected
  if (deviceConnected) {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastDataSend >= DATA_SEND_INTERVAL) {
      updateHealthData();  // Update simulated data
      sendAllData();       // Send to app
      lastDataSend = currentMillis;
    }
  }
  
  // Small delay to prevent overwhelming the CPU
  delay(10);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void printStatus() {
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("📊 CURRENT STATUS");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("📱 Connected: %s\n", deviceConnected ? "YES" : "NO");
  Serial.printf("👣 Steps: %d\n", stepCount);
  Serial.printf("💓 Heart Rate: %d BPM\n", heartRate);
  Serial.printf("🔋 Battery: %d%%\n", batteryLevel);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}
