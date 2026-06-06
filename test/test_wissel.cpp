#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"

// Wissel timing constants — keep in sync with Wissel.cpp
static const unsigned long PULS_MS     = 125;
static const unsigned long SETTLING_MS = 25;

// ── Fixture ───────────────────────────────────────────────────────────────────
// A Wissel drives two output pins: one for rechtdoor, one for afbuigend.
// Only one may be HIGH at a time (CDU pulse).  The static semaphore Wissel::actief
// ensures only one wissel pulses at a time across the whole yard.
// resetCDU() clears that semaphore at the start of each test so a leftover
// dangling pointer from the previous test case cannot block pulsing.

static bool _ladderVrij = true;
static bool ladderGuard() { return _ladderVrij; }

struct WisselFixture {
    VirtualIO rechtdoor, afbuigend;
    Wissel    wissel;

    WisselFixture() : wissel(&rechtdoor, &afbuigend, 1) {
        resetTime();
        _ladderVrij = true;
        Wissel::resetCDU();               // clear semaphore left by previous test
        Wissel::setIsLadderVrijFn(ladderGuard);
        wissel.init();
    }

    // Drive one complete CDU cycle: idle→pulsing→settling→idle.
    // Call this after activate() to confirm the direction in richting.
    void completeCycle() {
        wissel.update();                    // idle → pulsing (pin goes HIGH)
        advanceTime(PULS_MS);
        wissel.update();                    // pulsing → settling (pin goes LOW)
        advanceTime(SETTLING_MS);
        wissel.update();                    // settling → idle (richting confirmed)
    }
};

// ── Direction queuing ─────────────────────────────────────────────────────────

TEST_CASE("wissel: activate rechtdoor fires on next update") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.rechtdoor.getValue() == HIGH);  // pulse started
}

TEST_CASE("wissel: activate afbuigend fires correct pin") {
    WisselFixture f;
    f.wissel.activate(Richting::afbuigend);
    f.wissel.update();
    REQUIRE(f.afbuigend.getValue() == HIGH);
    REQUIRE(f.rechtdoor.getValue() == LOW);  // only one pin at a time
}

TEST_CASE("wissel: same direction as current → no pulse") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.completeCycle();
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);

    // Requesting the same direction again must not trigger another CDU pulse
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy() == false);
}

// ── Pulse timing ──────────────────────────────────────────────────────────────

TEST_CASE("wissel: pin goes LOW after PULS_MS") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();  // start pulse
    REQUIRE(f.rechtdoor.getValue() == HIGH);

    advanceTime(PULS_MS);
    f.wissel.update();  // end pulse — pin must drop to protect the motor coil
    REQUIRE(f.rechtdoor.getValue() == LOW);
}

TEST_CASE("wissel: direction confirmed after settling") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.completeCycle();
    // richting is only committed after settling; mechanical movement needs time
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);
    REQUIRE(f.wissel.isBusy()      == false);
}

TEST_CASE("wissel: isBusy during pulse and settling") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy());  // pulsing

    advanceTime(PULS_MS);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy());  // settling: pin low but motor may still be moving

    advanceTime(SETTLING_MS);
    f.wissel.update();
    REQUIRE_FALSE(f.wissel.isBusy());
}

// ── Ladder guard ──────────────────────────────────────────────────────────────
// The ladder guard prevents pulsing while another train is on the ladder.
// It only blocks the idle→pulsing transition; an active pulse is never interrupted.

TEST_CASE("wissel: does not fire while ladder busy") {
    WisselFixture f;
    _ladderVrij = false;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.rechtdoor.getValue() == LOW);   // held off
    REQUIRE_FALSE(f.wissel.isBusy());
}

TEST_CASE("wissel: fires after ladder becomes free") {
    WisselFixture f;
    _ladderVrij = false;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE_FALSE(f.wissel.isBusy());

    _ladderVrij = true;
    f.wissel.update();  // now allowed to acquire CDU
    REQUIRE(f.wissel.isBusy());
    REQUIRE(f.rechtdoor.getValue() == HIGH);
}

// ── CDU semaphore ─────────────────────────────────────────────────────────────
// Two wissels share one CDU (capacitor-discharge unit).  Only the first to acquire
// the semaphore may pulse; the second must wait until the first is fully settled.

TEST_CASE("wissel: semaphore — second wissel waits while first is pulsing") {
    VirtualIO rA, aA, rB, aB;
    Wissel w1(&rA, &aA, 1);
    Wissel w2(&rB, &aB, 2);

    resetTime();
    _ladderVrij = true;
    Wissel::setIsLadderVrijFn(ladderGuard);
    w1.init();
    w2.init();

    w1.activate(Richting::rechtdoor);
    w2.activate(Richting::rechtdoor);

    w1.update();
    w2.update();

    REQUIRE(rA.getValue() == HIGH);  // w1 acquired CDU, pulsing
    REQUIRE(rB.getValue() == LOW);   // w2 blocked — cannot fire while w1 is active

    // Let w1 finish its full cycle
    advanceTime(PULS_MS);
    w1.update(); w2.update();
    advanceTime(SETTLING_MS);
    w1.update(); w2.update();

    // w1 released the semaphore; w2 may now fire
    w2.update();
    REQUIRE(rB.getValue() == HIGH);
}

// ── Noop wissel ───────────────────────────────────────────────────────────────
// A Wissel constructed without pins (the default constructor) represents a track
// position that needs no physical turnout — activate() confirms the direction
// instantly without any CDU pulse.

TEST_CASE("wissel: noop (no pins) confirms direction immediately") {
    Wissel noop;
    resetTime();
    noop.activate(Richting::rechtdoor);
    REQUIRE(noop.getRichting() == Richting::rechtdoor);
    REQUIRE_FALSE(noop.isBusy());
}
