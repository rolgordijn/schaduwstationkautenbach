#include "IODebugMessage.h"
#include "Arduino.h"

// Constructor that takes the device name and state labels
IODebugMessage::IODebugMessage(const char* deviceName, const char* onLabel = "is aan", const char* offLabel = "is uit") {
  // Initialize the state labels
  strncpy(stateOnLabel, onLabel, sizeof(stateOnLabel) - 1);
  stateOnLabel[sizeof(stateOnLabel) - 1] = '\0';  // Ensure null termination

  strncpy(stateOffLabel, offLabel, sizeof(stateOffLabel) - 1);
  stateOffLabel[sizeof(stateOffLabel) - 1] = '\0';  // Ensure null termination

  strncpy(name, deviceName, sizeof(name) - 1);
  stateOffLabel[sizeof(name) - 1] = '\0';  // Ensure null termination

  // Initialize with the off state message
  snprintf(message, sizeof(message), "%s %s", deviceName, stateOffLabel);
}

// Method to update the message based on the current state
void IODebugMessage::updateMessage(bool currentState) {
  snprintf(message, sizeof(message), "%s %s", name, currentState ? stateOnLabel : stateOffLabel);
}

// Method to get the message
const char* IODebugMessage::getMessage() const {
  return message;
}
