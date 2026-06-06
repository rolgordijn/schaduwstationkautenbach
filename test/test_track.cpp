#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Track.h"
#include "constants.h"

// ── Fixture ───────────────────────────────────────────────────────────────────
// Each Track needs: a relay (powers the track), a departure button, an occupancy
// sensor, a turnout, an LED, a blink timer, and an exit sensor (detects the tail
// clearing the yard throat).  All are VirtualIO so we can read/write them in tests.
//
// The Wissel is a no-pin "noop" instance — activate() confirms the direction
// instantly without touching CDU hardware, so wissel direction changes are free.
//
// initAndTick() mirrors the real boot sequence: init() resets outputs to LOW and
// sets status=initialisatie, then the first tick resolves that to vrij or bezet
// based on the sensor.  applyOutputs (LED/relay) is NOT called during the
// initialisatie tick — it runs on the first tick already IN the target state,
// so tests that check outputs need one extra tick after reaching the target state.

static bool ladderVrij() { return true; }

struct TrackFixture {
    VirtualIO relay, knop, sensor, led, exitSensor;
    Wissel    wissel;   // noop: no CDU pulse, direction confirmed instantly
    Knipper   knipper;  // default 500/1000 ms — safe for millis()%period
    Track     track;

    TrackFixture()
        : knop(KNOP_NIET_INGEDUWD)  // active-low: HIGH = not pressed
        , sensor(VRIJ)
        , exitSensor(VRIJ)           // start clear so vertrek doesn't immediately detect exit
        , track(relay, knop, sensor, wissel, led, knipper, 0)
    {
        resetTime();
        Wissel::setIsLadderVrijFn(ladderVrij);
    }

    void tick(bool magVertrekken = true) {
        wissel.update();
        track.update(exitSensor, magVertrekken);
    }

    void initAndTick() {
        track.init();
        tick();  // resolves initialisatie → vrij or bezet; outputs still at init() LOW
    }
};

// ── Initialisation ────────────────────────────────────────────────────────────

TEST_CASE("track: init with vrij sensor → status vrij") {
    TrackFixture f;
    f.sensor.setValue(VRIJ);
    f.initAndTick();
    REQUIRE(f.track.getStatus() == TrackStatus::vrij);
}

TEST_CASE("track: init with bezet sensor → status bezet") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    REQUIRE(f.track.getStatus() == TrackStatus::bezet);
}

// ── vrij state ────────────────────────────────────────────────────────────────

TEST_CASE("track: vrij → bezet when sensor fires") {
    TrackFixture f;
    f.initAndTick();
    REQUIRE(f.track.getStatus() == TrackStatus::vrij);

    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::bezet);
}

TEST_CASE("track: vrij → relay and led are off") {
    TrackFixture f;
    f.initAndTick();
    // init() sets both outputs LOW; vrij's applyOutputs also sets them OFF — consistent
    REQUIRE(f.relay.getValue() == RELAY_OFF);
    REQUIRE(f.led.getValue()   == LED_OFF);
}

// ── bezet state ───────────────────────────────────────────────────────────────

TEST_CASE("track: bezet → led on, relay off") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();  // tick 1: initialisatie → bezet; outputs still LOW from init()
    f.tick();          // tick 2: bezet case runs applyOutputs(LED_ON, RELAY_OFF)
    REQUIRE(f.led.getValue()   == LED_ON);
    REQUIRE(f.relay.getValue() == RELAY_OFF);
}

TEST_CASE("track: bezet + button + ladder free → vertrek") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();

    f.knop.setValue(KNOP_INGEDUWD);
    f.tick(/*magVertrekken=*/true);
    REQUIRE(f.track.getStatus() == TrackStatus::vertrek);
}

TEST_CASE("track: bezet + button but ladder busy → stays bezet") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();

    f.knop.setValue(KNOP_INGEDUWD);
    f.tick(/*magVertrekken=*/false);  // ladder busy: departure not allowed
    REQUIRE(f.track.getStatus() == TrackStatus::bezet);
}

TEST_CASE("track: bezet + no button → stays bezet") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::bezet);
}

TEST_CASE("track: didStatusChange fires on vrij→bezet transition") {
    TrackFixture f;
    f.initAndTick();
    f.track.clearStatusChange();  // reset the flag after boot transition

    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.didStatusChange());
    REQUIRE(f.track.getStatus() == TrackStatus::bezet);
}

// ── vertrek state ─────────────────────────────────────────────────────────────

TEST_CASE("track: vertrek → relay on") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();  // bezet runs applyOutputs(RELAY_OFF) then transitions to vertrek
    REQUIRE(f.track.getStatus() == TrackStatus::vertrek);
    f.tick();  // now IN vertrek: applyOutputs(knipper, RELAY_ON) fires
    REQUIRE(f.relay.getValue()  == RELAY_ON);
}

TEST_CASE("track: vertrek + exit sensor bezet → vertrekGedetecteerd") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();  // → vertrek

    f.exitSensor.setValue(BEZET);  // front of train reaches the yard throat
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::vertrekGedetecteerd);
}

// ── vertrekGedetecteerd state ─────────────────────────────────────────────────

TEST_CASE("track: vertrekGedetecteerd + exit sensor clears → vrij") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();                         // → vertrek
    f.exitSensor.setValue(BEZET);
    f.tick();                         // → vertrekGedetecteerd
    REQUIRE(f.track.getStatus() == TrackStatus::vertrekGedetecteerd);

    f.exitSensor.setValue(VRIJ);  // tail has cleared
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::vrij);
}

TEST_CASE("track: vertrekGedetecteerd → relay stays on until exit clears") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();                     // → vertrek (relay set ON here via applyOutputs)
    f.exitSensor.setValue(BEZET);
    f.tick();                     // → vertrekGedetecteerd; relay stays ON to keep last wagons moving
    REQUIRE(f.relay.getValue() == RELAY_ON);
}

// ── triggerVertrek ────────────────────────────────────────────────────────────
// triggerVertrek() is the software equivalent of pressing the departure button
// (used by Autopilot and the web interface).

TEST_CASE("track: triggerVertrek on bezet → vertrek") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();

    f.track.triggerVertrek();
    REQUIRE(f.track.getStatus() == TrackStatus::vertrek);
}

TEST_CASE("track: triggerVertrek on vrij → no effect") {
    TrackFixture f;
    f.initAndTick();
    f.track.triggerVertrek();  // nothing to depart
    REQUIRE(f.track.getStatus() == TrackStatus::vrij);
}

// ── isVrij / isVertrekkend ────────────────────────────────────────────────────

TEST_CASE("track: isVrij correct across states") {
    TrackFixture f;
    f.initAndTick();
    REQUIRE(f.track.isVrij());

    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE_FALSE(f.track.isVrij());
}

TEST_CASE("track: isVertrekkend true in vertrek and vertrekGedetecteerd") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.track.isVertrekkend());  // true in vertrek

    f.exitSensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.isVertrekkend());  // still true in vertrekGedetecteerd — relay must stay on

    f.exitSensor.setValue(VRIJ);
    f.tick();
    REQUIRE_FALSE(f.track.isVertrekkend());  // vrij: departure complete
}
