#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Track.h"
#include "constants.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

static bool ladderVrij() { return true; }

struct TrackFixture {
    VirtualIO relay, knop, sensor, led, exitSensor;
    Wissel    wissel;   // wisselNoop — no hardware pins
    Knipper   knipper;
    Track     track;

    TrackFixture()
        : knop(KNOP_NIET_INGEDUWD)
        , sensor(VRIJ)
        , exitSensor(VRIJ)
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
        tick();
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
    REQUIRE(f.relay.getValue() == RELAY_OFF);
    REQUIRE(f.led.getValue()   == LED_OFF);
}

// ── bezet state ───────────────────────────────────────────────────────────────

TEST_CASE("track: bezet → led on, relay off") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();  // initialisatie → bezet (no applyOutputs yet)
    f.tick();          // bezet: applyOutputs(LED_ON, RELAY_OFF)
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
    f.tick(/*magVertrekken=*/false);
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
    f.track.clearStatusChange();

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
    f.tick();  // bezet → vertrek (relay still RELAY_OFF from bezet's applyOutputs)
    REQUIRE(f.track.getStatus() == TrackStatus::vertrek);
    f.tick();  // vertrek: applyOutputs(knipper, RELAY_ON)
    REQUIRE(f.relay.getValue()  == RELAY_ON);
}

TEST_CASE("track: vertrek + exit sensor bezet → vertrekGedetecteerd") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();

    f.exitSensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::vertrekGedetecteerd);
}

// ── vertrekGedetecteerd state ─────────────────────────────────────────────────

TEST_CASE("track: vertrekGedetecteerd + exit sensor clears → vrij") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();
    f.exitSensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::vertrekGedetecteerd);

    f.exitSensor.setValue(VRIJ);
    f.tick();
    REQUIRE(f.track.getStatus() == TrackStatus::vrij);
}

TEST_CASE("track: vertrekGedetecteerd → relay stays on until exit clears") {
    TrackFixture f;
    f.sensor.setValue(BEZET);
    f.initAndTick();
    f.knop.setValue(KNOP_INGEDUWD);
    f.tick();
    f.exitSensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.relay.getValue() == RELAY_ON);
}

// ── triggerVertrek ────────────────────────────────────────────────────────────

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
    f.track.triggerVertrek();
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
    REQUIRE(f.track.isVertrekkend());

    f.exitSensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.track.isVertrekkend());

    f.exitSensor.setValue(VRIJ);
    f.tick();
    REQUIRE_FALSE(f.track.isVertrekkend());
}
