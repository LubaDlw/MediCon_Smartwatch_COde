import serial
import time
import re

try:
    # Connect to COM3 at 115200 baud
    ser = serial.Serial('COM3', 115200, timeout=0.5)
    print("Connected to COM3 at 115200 baud")
    print("=" * 60)
    print("PRESS THE RESET BUTTON ON YOUR ESP32 NOW!")
    print("=" * 60)
    print("Waiting for boot messages and IP address...")
    print("(Press Ctrl+C to stop)")
    print()
    
    ip_found = False
    ip_pattern = re.compile(r'\b(?:\d{1,3}\.){3}\d{1,3}\b')
    
    # Read continuously
    try:
        while not ip_found:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(line)
                    
                    # Look for IP address patterns
                    if "IP Address" in line or "IP:" in line or ip_pattern.search(line):
                        ips = ip_pattern.findall(line)
                        if ips:
                            print("\n" + "=" * 60)
                            print("✅ FOUND IP ADDRESS!")
                            print("=" * 60)
                            for ip in ips:
                                # Filter out common non-IP patterns
                                if not ip.startswith('0.') and not ip.startswith('255.'):
                                    print(f"📡 Your ESP32 IP Address: {ip}")
                                    print("=" * 60)
                                    print(f"\nEnter this in your Expo app: {ip}")
                                    ip_found = True
                            print()
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n\nStopped by user.")
    
    ser.close()
    print("\nConnection closed.")
    
except serial.SerialException as e:
    print(f"Error: {e}")
    print("\nTroubleshooting:")
    print("1. Make sure the ESP32 is connected via USB")
    print("2. Check if another program is using COM3 (close Arduino IDE, PlatformIO monitor, etc.)")
    print("3. Try unplugging and reconnecting the device")
except ModuleNotFoundError:
    print("Error: pyserial not installed")
    print("Install it with: python -m pip install pyserial")
