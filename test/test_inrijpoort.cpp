#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Knipper.h"
#include "InrijPoort.h"
#include "constants.h"

// ── Fixture ───────────────────────────────────────────────────────────────────
// InrijPoort (entry gate) controls whether arriving trains may enter the yard.
//
// sensor    — occupancy detector at the entry section (BEZET = train present)
// relais    — relay that powers/enables the entry track (RELAY_ON = trains may pass)
// led       — status LED on the entry side
// sensorUit — occupancy detector at the exit side (read-only in these tests)
// ledUit    — status LED on the exit side
//
// The gate logic:
//   • When sensor goes BEZET and the yard has room: gate opens (relay ON) for
//     INRIJ_VERTRAGING_MS so the loco clears the relay-controlled section.
//   • After INRIJ_VERTRAGING_MS the relay stays ON while sensor is clear; if sensor
//     is still BEZET a second train is waiting and the relay drops to block it.
//   • treinAangekomen() closes the gate early once the operator confirms arrival,
//     but only after INRIJ_VERTRAGING_MS has elapsed (so the loco has cleared).
//   • When yardVol=true the relay is OFF (no space — block everything).

struct InrijFixture {
    VirtualIO sensor, relais, led, sensorUit, ledUit;
    Knipper   knipper;  // default 500/1000 ms — safe for millis()%period
    InrijPoort poort;

    InrijFixture()
        : sensor(VRIJ)
        , poort(sensor, relais, led, sensorUit, ledUit, knipper)
    {
        resetTime();
        poort.init();  // sets relais and led to LOW; sensor lines to input
    }

    void tick(bool yardVol = false, bool kopspoorInRijden = false) {
        poort.update(yardVol, kopspoorInRijden);
    }
};

// ── Initial state ─────────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: not traversing initially") {
    InrijFixture f;
    REQUIRE_FALSE(f.poort.isTraversing());  // gate cycle not started
}

TEST_CASE("inrijpoort: relay on when yard not full (trains may enter)") {
    InrijFixture f;
    f.tick();
    // With space in the yard and no active gate cycle, the relay is ON so trains
    // can roll in freely; it only drops to block when the yard is full.
    REQUIRE(f.relais.getValue() == RELAY_ON);
}

// ── Gate activation ───────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: sensor bezet activates gate (relay on)") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_ON);
    REQUIRE(f.poort.isTraversing());
}

TEST_CASE("inrijpoort: relay stays on during INRIJ_VERTRAGING_MS") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();  // gate starts, forceAan=true

    // One millisecond before the delay expires: relay must still be ON
    advanceTime(INRIJ_VERTRAGING_MS - 1);
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_ON);
}

TEST_CASE("inrijpoort: relay turns off after INRIJ_VERTRAGING_MS") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();  // gate starts

    // After the delay the loco has cleared the relay section; drop relay to
    // block any second train still sitting at the entry sensor.
    advanceTime(INRIJ_VERTRAGING_MS);
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_OFF);
}

// ── Gate close via treinAangekomen ────────────────────────────────────────────

TEST_CASE("inrijpoort: treinAangekomen closes gate") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();  // gate starts at t=0
    REQUIRE(f.poort.isTraversing());

    // treinAangekomen() is ignored before INRIJ_VERTRAGING_MS — the loco may
    // still be on the relay-controlled section.  Also: sensor must be VRIJ when
    // treinAangekomen() is called, otherwise a second-train chain is assumed and
    // the gate restarts immediately.
    advanceTime(INRIJ_VERTRAGING_MS);
    f.sensor.setValue(VRIJ);  // loco has reached its track, entry section clear
    f.tick();                 // updates inrijBezet = false
    f.poort.treinAangekomen();
    REQUIRE_FALSE(f.poort.isTraversing());
}

// ── Chain detection ───────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: second train waiting restarts gate immediately") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();

    // Operator confirms first train arrived, but sensor is still BEZET →
    // a second train is waiting; the gate restarts for it right away.
    f.poort.treinAangekomen();
    f.tick();  // sensor still BEZET — chain detected
    REQUIRE(f.poort.isTraversing());
}

// ── Yard vol blocks arrival ────────────────────────────────────────────────────

TEST_CASE("inrijpoort: gate does not activate when yard is vol") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick(/*yardVol=*/true);  // no free tracks — do not open the gate
    REQUIRE(f.relais.getValue() == RELAY_OFF);
}

// ── Safety timeout ────────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: safety timeout closes gate if no arrival detected") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.poort.isTraversing());

    // If treinAangekomen() is never called the gate forces itself closed after
    // INRIJ_TIMEOUT_MS to prevent the entry from staying open indefinitely.
    advanceTime(INRIJ_TIMEOUT_MS);
    f.tick();
    REQUIRE_FALSE(f.poort.isTraversing());
}
