#pragma once 

class IODebugMessage {
private:
    char message[64];      // 15 (name) + 1 (space) + 7 (state) + 1 (null terminator)
    char name[32]; 
    char stateOnLabel[32];  // Max 7 characters + 1 for null terminator
    char stateOffLabel[32]; // Max 7 characters + 1 for null terminator

public:
    // Constructor that takes the device name and state labels
    IODebugMessage(const char* deviceName, const char* onLabel, const char* offLabel); 
    // Method to update the message based on the current state
    void updateMessage(bool currentState); 

    // Method to get the message
    const char* getMessage() const;
};