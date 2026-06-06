#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Track.h"
#include "Autopilot.h"
#include "constants.h"

static bool _apLadderVrij = true;
static bool apLadderVrij() { return _apLadderVrij; }

struct AutopilotFixture {
    VirtualIO schakelaar;

    // Two tracks with their own IOs
    VirtualIO relay1, knop1, sensor1, led1;
    VirtualIO relay2, knop2, sensor2, led2;
    Wissel    w1, w2;
    Knipper   knipper;
    Track     t1, t2;
    Track*    sporen[2];

    Autopilot autopilot;

    AutopilotFixture()
        : schakelaar(KNOP_NIET_INGEDUWD)
        , knop1(true), knop2(true)
        , sensor1(VRIJ), sensor2(VRIJ)
        , t1(relay1, knop1, sensor1, w1, led1, knipper, 0)
        , t2(relay2, knop2, sensor2, w2, led2, knipper, 1)
        , autopilot(schakelaar, sporen, 2)
    {
        sporen[0] = &t1;
        sporen[1] = &t2;
        resetTime();
        _apLadderVrij = true;
        Wissel::setIsLadderVrijFn(apLadderVrij);
        t1.init(); t1.update(sensor1, true);  // → vrij
        t2.init(); t2.update(sensor2, true);  // → vrij
        autopilot.init();
    }

    void setBezet(Track& t, VirtualIO& sensor) {
        sensor.setValue(BEZET);
        VirtualIO exit;
        t.update(exit, true);
    }
};

// ── Auto/manual mode ──────────────────────────────────────────────────────────

TEST_CASE("autopilot: not active in manual mode") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_NIET_INGEDUWD);
    REQUIRE_FALSE(f.autopilot.isActief());
}

TEST_CASE("autopilot: active when schakelaar ingeduwd") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    REQUIRE(f.autopilot.isActief());
}

TEST_CASE("autopilot: switching to manual clears queue") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);
    f.autopilot.treinAangekomen(0, false);  // queue it
    REQUIRE(f.autopilot.getWachtrij() == 1);

    f.schakelaar.setValue(KNOP_NIET_INGEDUWD);
    f.autopilot.update(true);
    REQUIRE(f.autopilot.getWachtrij() == 0);
}

// ── Immediate departure ───────────────────────────────────────────────────────

TEST_CASE("autopilot: treinAangekomen fires immediately when interval elapsed") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);

    advanceTime(VERTREK_INTERVAL_MS);
    f.autopilot.treinAangekomen(0, /*magVertrekken=*/true);

    // Queue should be 0 — departure fired immediately, not queued
    REQUIRE(f.autopilot.getWachtrij() == 0);
}

// ── Queuing ───────────────────────────────────────────────────────────────────

TEST_CASE("autopilot: treinAangekomen queues when interval not elapsed") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);

    // Time has not elapsed — should queue
    f.autopilot.treinAangekomen(0, /*magVertrekken=*/true);
    REQUIRE(f.autopilot.getWachtrij() == 1);
}

TEST_CASE("autopilot: update dequeues after interval") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);
    f.autopilot.treinAangekomen(0, false);
    REQUIRE(f.autopilot.getWachtrij() == 1);

    advanceTime(VERTREK_INTERVAL_MS);
    f.autopilot.update(/*magVertrekken=*/true);
    REQUIRE(f.autopilot.getWachtrij() == 0);
    REQUIRE(f.t1.getStatus() == TrackStatus::vertrek);
}

TEST_CASE("autopilot: update does not dequeue when ladder busy") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);
    f.autopilot.treinAangekomen(0, false);

    advanceTime(VERTREK_INTERVAL_MS);
    f.autopilot.update(/*magVertrekken=*/false);
    REQUIRE(f.autopilot.getWachtrij() == 1);
}
