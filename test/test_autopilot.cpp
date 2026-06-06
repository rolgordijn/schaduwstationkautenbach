#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Track.h"
#include "Autopilot.h"
#include "constants.h"

// ── Fixture ───────────────────────────────────────────────────────────────────
// Autopilot manages automatic departures when the operator is not at the panel.
// It watches a mode switch (schakelaar) and a queue of arriving trains.
//
// A departure fires only when:
//   1. Autopilot is active (schakelaar ingeduwd)
//   2. The target track is bezet
//   3. VERTREK_INTERVAL_MS has elapsed since the last departure
//   4. The ladder is free (magVertrekken)
//
// The fixture uses two tracks with noop wissels (no CDU, instant direction
// confirm) so wissel timing does not affect autopilot logic tests.

static bool _apLadderVrij = true;
static bool apLadderVrij() { return _apLadderVrij; }

struct AutopilotFixture {
    VirtualIO schakelaar;

    VirtualIO relay1, knop1, sensor1, led1;
    VirtualIO relay2, knop2, sensor2, led2;
    Wissel    w1, w2;   // noop: no pins, direction confirmed instantly
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
        t1.init(); t1.update(sensor1, true);  // boot both tracks to vrij
        t2.init(); t2.update(sensor2, true);
        autopilot.init();
    }

    // Simulate a train arriving on track t by asserting its sensor.
    // Uses a throwaway exit sensor (VRIJ) — we only need the bezet state here.
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
    f.autopilot.treinAangekomen(0, false);  // queue track 0 (interval not yet elapsed)
    REQUIRE(f.autopilot.getWachtrij() == 1);

    // Operator takes manual control — pending departure must be cancelled
    f.schakelaar.setValue(KNOP_NIET_INGEDUWD);
    f.autopilot.update(true);
    REQUIRE(f.autopilot.getWachtrij() == 0);
}

// ── Immediate departure ───────────────────────────────────────────────────────

TEST_CASE("autopilot: treinAangekomen fires immediately when interval elapsed") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);

    // If the minimum interval has already passed, the departure triggers at once
    // and nothing is queued.
    advanceTime(VERTREK_INTERVAL_MS);
    f.autopilot.treinAangekomen(0, /*magVertrekken=*/true);
    REQUIRE(f.autopilot.getWachtrij() == 0);
}

// ── Queuing ───────────────────────────────────────────────────────────────────

TEST_CASE("autopilot: treinAangekomen queues when interval not elapsed") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);

    // Interval has not elapsed yet — departure must be held in the queue
    f.autopilot.treinAangekomen(0, /*magVertrekken=*/true);
    REQUIRE(f.autopilot.getWachtrij() == 1);
}

TEST_CASE("autopilot: update dequeues after interval") {
    AutopilotFixture f;
    f.schakelaar.setValue(KNOP_INGEDUWD);
    f.setBezet(f.t1, f.sensor1);
    f.autopilot.treinAangekomen(0, false);  // queue it
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
    // Ladder busy (e.g. another train already departing): must not fire
    f.autopilot.update(/*magVertrekken=*/false);
    REQUIRE(f.autopilot.getWachtrij() == 1);
}
