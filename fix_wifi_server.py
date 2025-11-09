import re

# Read the file
with open('hal/esp32/wifi_server.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Replace all occurrences of WiFiServer:: with WatchWiFiServer::
# But not WatchWiFiServer::WatchWiFiServer (already fixed)
content = re.sub(r'\bWiFiServer::', 'WatchWiFiServer::', content)

# Write back to the file
with open('hal/esp32/wifi_server.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Successfully replaced all WiFiServer:: with WatchWiFiServer::")
