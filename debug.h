#pragma once

#include <HardwareSerial.h>

#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x); Serial.flush();
#define debugln(x) Serial.println(x);  Serial.flush();
#else
#define debug(x);
#define debugln(x);
#endif

/**
 

#define DEBUG 1

#define debug(x) { if (DEBUG) { Serial.print(F(__FILE__)); Serial.print(F(":"));
Serial.print(__LINE__); Serial.print(F(":")); Serial.print(F(__FUNCTION__)); Serial.print(F(":")); Serial.print(x); Serial.flush();
}
}
#define debugln(x) { if (DEBUG) { Serial.print(F(__FILE__)); Serial.print(F(":")); Serial.print(__LINE__); Serial.print(F(":")); Serial.print(F(__FUNCTION__)); Serial.print(F(":")); Serial.println(x); Serial.flush(); } }


*/
