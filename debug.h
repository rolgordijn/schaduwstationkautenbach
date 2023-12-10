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

