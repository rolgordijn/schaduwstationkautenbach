#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ── Pin constants ─────────────────────────────────────────────────────────────
#define HIGH         1
#define LOW          0
#define INPUT        0
#define OUTPUT       1
#define INPUT_PULLUP 2

// ── Flash string macro — no-op on host ───────────────────────────────────────
#define F(x) (x)

// ── Controllable clock ────────────────────────────────────────────────────────
extern unsigned long _fakeMillis;
inline unsigned long millis()                  { return _fakeMillis; }
inline void          advanceTime(unsigned long ms) { _fakeMillis += ms; }
inline void          resetTime()               { _fakeMillis = 0; }

// ── Arduino stubs (only needed when BasicIO compiles — not in test builds) ───
inline void digitalWrite(int, int) {}
inline int  digitalRead(int)       { return HIGH; }
inline void pinMode(int, int)      {}

// ── random ────────────────────────────────────────────────────────────────────
inline long random(long max)            { return max > 0 ? 0 : 0; }
inline long random(long /*min*/, long /*max*/) { return 0; }

// ── Minimal Serial stub ───────────────────────────────────────────────────────
struct _SerialStub {
    void begin(long)            {}
    void print(const char*)     {}
    void print(long)            {}
    void println(const char*)   {}
    void println(long)          {}
    void flush()                {}
};
extern _SerialStub Serial;
