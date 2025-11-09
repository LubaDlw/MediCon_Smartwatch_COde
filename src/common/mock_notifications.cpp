/*
   MIT License

  Mock Chronic Health Management Notification System Implementation
*/

#include "mock_notifications.h"

// Mock database of chronic health management messages
static const char* HEALTH_MESSAGES[] = {
    "It's time to take your blood pressure medication.",
    "Don't forget your morning insulin dose.",
    "Remember to check your glucose levels.",
    "Take your cholesterol medication now.",
    "Have you taken your arthritis medication today?",
    "Time for your daily inhaler dose.",
    "It's time to take your evening pills.",
    "Log your medication intake in the app.",
    "Stay hydrated to support your treatment.",
    "Remember to monitor your heart rate.",
    "Time to check your blood sugar levels.",
    "Don't forget your vitamin D supplement.",
    "Take a short walk to improve circulation.",
    "Remember to take your thyroid medication.",
    "Time for your blood pressure check.",
    "Have you taken your pain medication?",
    "Stay consistent with your medication schedule.",
    "Remember to rest and manage your stress.",
    "Time to log your daily health metrics.",
    "Don't skip your prescribed medication."
};

static const int NUM_HEALTH_MESSAGES = sizeof(HEALTH_MESSAGES) / sizeof(HEALTH_MESSAGES[0]);

// Notification history (circular buffer)
#define MAX_NOTIFICATION_HISTORY 20
static MockNotification notification_history[MAX_NOTIFICATION_HISTORY];
static int notification_count = 0;
static int current_index = 0;

// Timer state
static unsigned long last_notification_time = 0;
static const unsigned long NOTIFICATION_INTERVAL = 45000; // 45 seconds in milliseconds

// New notification flag
static bool has_new_notification = false;

// Helper function to get current time string
static String getCurrentTimeString() {
    // Simple time format - hours:minutes
    unsigned long currentMillis = millis();
    unsigned long totalSeconds = currentMillis / 1000;
    int hours = (totalSeconds / 3600) % 24;
    int minutes = (totalSeconds / 60) % 60;
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hours, minutes);
    return String(timeStr);
}

// Helper function to get random message index
static int getRandomMessageIndex() {
    return random(0, NUM_HEALTH_MESSAGES);
}

// Helper function to add notification to history
static void addNotificationToHistory(int icon, const char* message) {
    // Add to circular buffer
    int writeIndex = notification_count % MAX_NOTIFICATION_HISTORY;
    notification_history[writeIndex].icon = icon;
    notification_history[writeIndex].message = message;
    notification_history[writeIndex].time = getCurrentTimeString();
    
    notification_count++;
    if (notification_count > MAX_NOTIFICATION_HISTORY) {
        notification_count = MAX_NOTIFICATION_HISTORY;
    }
}

void mock_notifications_init() {
    // Initialize random seed
    randomSeed(millis());
    
    // Reset state
    notification_count = 0;
    current_index = 0;
    has_new_notification = false;
    last_notification_time = millis();
    
    // Add initial notification
    int msgIndex = getRandomMessageIndex();
    addNotificationToHistory(22, HEALTH_MESSAGES[msgIndex]); // Icon 22 is a generic notification icon
    has_new_notification = true;
}

void mock_notifications_update() {
    unsigned long currentMillis = millis();
    
    // Check if it's time for a new notification
    if (currentMillis - last_notification_time >= NOTIFICATION_INTERVAL) {
        last_notification_time = currentMillis;
        
        // Generate new notification
        int msgIndex = getRandomMessageIndex();
        
        // Use different icons for variety (cycling through health-related icon indices)
        int iconIndex = 22 + (notification_count % 3); // Icons 22, 23, 24
        
        addNotificationToHistory(iconIndex, HEALTH_MESSAGES[msgIndex]);
        has_new_notification = true;
    }
}

MockNotification mock_notifications_get_current() {
    if (notification_count > 0) {
        // Return the most recent notification (last added)
        int index = (notification_count - 1) % MAX_NOTIFICATION_HISTORY;
        return notification_history[index];
    }
    
    // Return empty notification if none exist
    MockNotification empty;
    empty.icon = 22;
    empty.message = "No notifications";
    empty.time = getCurrentTimeString();
    return empty;
}

MockNotification mock_notifications_get_at(int index) {
    if (notification_count > 0 && index >= 0 && index < notification_count) {
        // Return notification from history (reverse order - newest first)
        int historyIndex = (notification_count - 1 - index) % MAX_NOTIFICATION_HISTORY;
        if (historyIndex < 0) historyIndex += MAX_NOTIFICATION_HISTORY;
        return notification_history[historyIndex];
    }
    
    // Return empty notification if index invalid
    MockNotification empty;
    empty.icon = 22;
    empty.message = "No notification at this index";
    empty.time = getCurrentTimeString();
    return empty;
}

int mock_notifications_get_count() {
    return notification_count;
}

bool mock_notifications_has_new() {
    return has_new_notification;
}

void mock_notifications_clear_new() {
    has_new_notification = false;
}
