#pragma once
#include <stdint.h>

// Stub — tests use VirtualIO instead of MCP23017IO so this class is never instantiated.
class MCP23017 {
public:
    void    begin(uint8_t)           {}
    void    pinMode(uint8_t, uint8_t){}
    void    digitalWrite(uint8_t, uint8_t) {}
    uint8_t digitalRead(uint8_t)     { return 1; }
};
