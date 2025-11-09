/*
   MIT License

  Mock Chronic Health Management Notification System
  
  This module provides mock notifications for chronic health management
  without requiring Bluetooth or Chronos app connectivity.
*/

#ifndef MOCK_NOTIFICATIONS_H
#define MOCK_NOTIFICATIONS_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Mock notification structure
typedef struct {
    int icon;
    const char* message;
    String time;
} MockNotification;

// Initialize the mock notification system
void mock_notifications_init();

// Update the mock notification system (call in main loop)
void mock_notifications_update();

// Get the current mock notification
MockNotification mock_notifications_get_current();

// Get mock notification at specific index
MockNotification mock_notifications_get_at(int index);

// Get total number of mock notifications in history
int mock_notifications_get_count();

// Check if a new notification is available
bool mock_notifications_has_new();

// Clear the new notification flag
void mock_notifications_clear_new();

#ifdef __cplusplus
}
#endif

#endif // MOCK_NOTIFICATIONS_H
