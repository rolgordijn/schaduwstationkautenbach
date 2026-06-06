#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"

static const unsigned long PULS_MS     = 125;
static const unsigned long SETTLING_MS = 25;

// ── Ladder guard helpers ──────────────────────────────────────────────────────

static bool _ladderVrij = true;
static bool ladderGuard() { return _ladderVrij; }

struct WisselFixture {
    VirtualIO rechtdoor, afbuigend;
    Wissel    wissel;

    WisselFixture() : wissel(&rechtdoor, &afbuigend, 1) {
        resetTime();
        _ladderVrij = true;
        Wissel::setIsLadderVrijFn(ladderGuard);
        wissel.init();
    }

    // Advance one full pulse + settling cycle
    void completeCycle() {
        wissel.update();                    // idle → pulsing
        advanceTime(PULS_MS);
        wissel.update();                    // pulsing → settling
        advanceTime(SETTLING_MS);
        wissel.update();                    // settling → idle
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
    REQUIRE(f.rechtdoor.getValue() == LOW);
}

TEST_CASE("wissel: same direction as current → no pulse") {
    WisselFixture f;
    f.completeCycle();  // confirm rechtdoor as initial direction
    // Actually: no activate called so richting stays onbekend.
    // Set rechtdoor and confirm it.
    f.wissel.activate(Richting::rechtdoor);
    f.completeCycle();
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);

    // Activate same direction again — should not pulse
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy() == false);
}

// ── Pulse timing ──────────────────────────────────────────────────────────────

TEST_CASE("wissel: pin goes LOW after PULS_MS") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();                      // start pulse
    REQUIRE(f.rechtdoor.getValue() == HIGH);

    advanceTime(PULS_MS);
    f.wissel.update();                      // end pulse
    REQUIRE(f.rechtdoor.getValue() == LOW);
}

TEST_CASE("wissel: direction confirmed after settling") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.completeCycle();
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);
    REQUIRE(f.wissel.isBusy()      == false);
}

TEST_CASE("wissel: isBusy during pulse and settling") {
    WisselFixture f;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy());

    advanceTime(PULS_MS);
    f.wissel.update();
    REQUIRE(f.wissel.isBusy());     // still in settling

    advanceTime(SETTLING_MS);
    f.wissel.update();
    REQUIRE_FALSE(f.wissel.isBusy());
}

// ── Ladder guard ──────────────────────────────────────────────────────────────

TEST_CASE("wissel: does not fire while ladder busy") {
    WisselFixture f;
    _ladderVrij = false;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE(f.rechtdoor.getValue() == LOW);  // no pulse
    REQUIRE_FALSE(f.wissel.isBusy());
}

TEST_CASE("wissel: fires after ladder becomes free") {
    WisselFixture f;
    _ladderVrij = false;
    f.wissel.activate(Richting::rechtdoor);
    f.wissel.update();
    REQUIRE_FALSE(f.wissel.isBusy());

    _ladderVrij = true;
    f.wissel.update();
    REQUIRE(f.wissel.isBusy());
    REQUIRE(f.rechtdoor.getValue() == HIGH);
}

// ── CDU semaphore ─────────────────────────────────────────────────────────────

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

    REQUIRE(rA.getValue() == HIGH);  // w1 pulsing
    REQUIRE(rB.getValue() == LOW);   // w2 blocked by semaphore

    // Complete w1 cycle
    advanceTime(PULS_MS);
    w1.update(); w2.update();
    advanceTime(SETTLING_MS);
    w1.update(); w2.update();

    // Now w2 should fire
    w2.update();
    REQUIRE(rB.getValue() == HIGH);
}

// ── Noop wissel ───────────────────────────────────────────────────────────────

TEST_CASE("wissel: noop (no pins) confirms direction immediately") {
    Wissel noop;  // default constructor — no pins
    resetTime();
    noop.activate(Richting::rechtdoor);
    REQUIRE(noop.getRichting() == Richting::rechtdoor);
    REQUIRE_FALSE(noop.isBusy());
}
