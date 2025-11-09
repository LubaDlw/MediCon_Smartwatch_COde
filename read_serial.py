import serial
import time

try:
    # Connect to COM3 at 115200 baud
    ser = serial.Serial('COM3', 115200, timeout=1)
    print("Connected to COM3 at 115200 baud")
    print("Reading serial output... (Press Ctrl+C to stop)")
    print("=" * 60)
    
    # Read for 10 seconds or until we find IP
    start_time = time.time()
    while time.time() - start_time < 10:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(line)
                # Check if line contains IP address
                if "IP Address" in line or "IP:" in line:
                    print("\n" + "=" * 60)
                    print("FOUND IP ADDRESS!")
                    print("=" * 60)
    
    ser.close()
    print("\nConnection closed.")
    
except serial.SerialException as e:
    print(f"Error: {e}")
    print("\nNote: If device is not responding, try:")
    print("1. Press the RESET button on your ESP32")
    print("2. Re-run this script")
except ModuleNotFoundError:
    print("Error: pyserial not installed")
    print("Install it with: pip install pyserial")
