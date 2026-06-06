#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Knipper.h"
#include "InrijPoort.h"
#include "constants.h"

struct InrijFixture {
    VirtualIO sensor, relais, led, sensorUit, ledUit;
    Knipper   knipper;
    InrijPoort poort;

    InrijFixture()
        : sensor(VRIJ)
        , poort(sensor, relais, led, sensorUit, ledUit, knipper)
    {
        resetTime();
        poort.init();
    }

    void tick(bool yardVol = false, bool kopspoorInRijden = false) {
        poort.update(yardVol, kopspoorInRijden);
    }
};

// ── Initial state ─────────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: not traversing initially") {
    InrijFixture f;
    REQUIRE_FALSE(f.poort.isTraversing());
}

TEST_CASE("inrijpoort: relay off initially") {
    InrijFixture f;
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_OFF);
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
    f.tick();

    advanceTime(INRIJ_VERTRAGING_MS - 1);
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_ON);
}

TEST_CASE("inrijpoort: relay turns off after INRIJ_VERTRAGING_MS") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();

    advanceTime(INRIJ_VERTRAGING_MS);
    f.tick();
    REQUIRE(f.relais.getValue() == RELAY_OFF);
}

// ── Gate close via treinAangekomen ────────────────────────────────────────────

TEST_CASE("inrijpoort: treinAangekomen closes gate") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.poort.isTraversing());

    f.poort.treinAangekomen();
    f.sensor.setValue(VRIJ);  // first train has moved to its track
    f.tick();
    REQUIRE_FALSE(f.poort.isTraversing());
}

// ── Chain detection ───────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: second train waiting restarts gate immediately") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();

    // First train arrives at its track; second train still at entry sensor
    f.poort.treinAangekomen();
    f.tick();  // sensor still BEZET — chain detected
    REQUIRE(f.poort.isTraversing());
}

// ── Yard vol blocks arrival ────────────────────────────────────────────────────

TEST_CASE("inrijpoort: gate does not activate when yard is vol") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick(/*yardVol=*/true);
    REQUIRE(f.relais.getValue() == RELAY_OFF);
}

// ── Safety timeout ────────────────────────────────────────────────────────────

TEST_CASE("inrijpoort: safety timeout closes gate if no arrival detected") {
    InrijFixture f;
    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.poort.isTraversing());

    advanceTime(INRIJ_TIMEOUT_MS);
    f.tick();
    REQUIRE_FALSE(f.poort.isTraversing());
}
