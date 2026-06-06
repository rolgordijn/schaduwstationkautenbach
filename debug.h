#pragma once

#include <HardwareSerial.h>

#if DEBUG == 1
inline void _debugPrintTs() {
    Serial.print(F("["));
    Serial.print(millis());
    Serial.print(F("] "));
}
#define debug(x)   Serial.print(x)
#define debugln(x) do { _debugPrintTs(); Serial.println(x); Serial.flush(); } while(0)
#else
#define debug(x)
#define debugln(x)
#endif
