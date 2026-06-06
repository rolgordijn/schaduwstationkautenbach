#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Kopspoor.h"
#include "constants.h"

// ── Fixture ───────────────────────────────────────────────────────────────────
// Kopspoor is a dead-end stub track at the far end of the yard.  It shares the
// last turnout (wissel6) with spoor6: afbuigend routes trains into spoor6,
// rechtdoor routes them into the kopspoor.
//
// The fixture creates a real Wissel with VirtualIO pins so we can observe CDU
// pulses and confirm direction.  resetCDU() is essential: the CDU semaphore is
// a static pointer that persists between test cases; a dangling pointer left by
// a previous test would silently block all new pulses.
//
// tick() drives wissel.update() and then kopspoor.update().  Note that
// kopspoor.update() also calls wissel.update() internally, so the wissel FSM
// advances twice per tick — important when calculating timing.

static bool _ksLadderVrij = true;
static bool ksLadderVrij() { return _ksLadderVrij; }

struct KopspoorFixture {
    VirtualIO sensor, relais, led;
    VirtualIO btnIn, btnAnnuleer, btnUit, btnAnnuleerUit;
    Knipper   knipper;  // default 500/1000 ms — safe for millis()%period
    VirtualIO wisselR, wisselA;  // physical pins for wissel6
    Wissel    wissel;
    Kopspoor  kopspoor;

    KopspoorFixture()
        : sensor(VRIJ)
        , btnIn(KNOP_NIET_INGEDUWD)
        , btnAnnuleer(KNOP_NIET_INGEDUWD)
        , btnUit(KNOP_NIET_INGEDUWD)
        , btnAnnuleerUit(KNOP_NIET_INGEDUWD)
        , wissel(&wisselR, &wisselA, 6)
        , kopspoor(sensor, relais, led,
                   btnIn, btnAnnuleer, btnUit, btnAnnuleerUit,
                   knipper, wissel)
    {
        resetTime();
        _ksLadderVrij = true;
        Wissel::resetCDU();               // clear any dangling semaphore from previous test
        Wissel::setIsLadderVrijFn(ksLadderVrij);
        kopspoor.init();
    }

    void tick() {
        wissel.update();
        kopspoor.update(ksLadderVrij);
    }
};

// Drive one full CDU pulse + settling cycle.
// Precondition: pulsing has already started (wissel.update() has been called
// and the idle→pulsing transition has fired).
// After this helper, richting is confirmed and actief is released.
static void completeWisselCycle(KopspoorFixture& f) {
    advanceTime(200);  // > PULS_MS (125 ms): ends the pulse
    f.tick();          // pulsing → settling
    advanceTime(50);   // > SETTLING_MS (25 ms): ends the settling
    f.tick();          // settling → idle; richting confirmed
}

// ── Initial state ─────────────────────────────────────────────────────────────

TEST_CASE("kopspoor: initial status is vrij") {
    KopspoorFixture f;
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
}

TEST_CASE("kopspoor: wissel defaults to afbuigend (spoor6 accessible)") {
    KopspoorFixture f;
    // vrij state calls activate(afbuigend) every tick — pulsing starts on tick 2
    // (tick 1 queues gewensteRichting; tick 2's first wissel.update finds it pending)
    f.tick();  // vrij: gewensteRichting = afbuigend
    f.tick();  // pulsing starts (CDU acquired)
    completeWisselCycle(f);
    REQUIRE(f.wissel.getRichting() == Richting::afbuigend);
}

// ── vrij → inRijden ───────────────────────────────────────────────────────────

TEST_CASE("kopspoor: btnIn pressed while vrij → inRijden") {
    KopspoorFixture f;
    f.tick();  // settle vrij

    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::inRijden);
}

TEST_CASE("kopspoor: wissel switches to rechtdoor during inRijden") {
    KopspoorFixture f;
    f.tick();   // vrij: activate(afbuigend) queued
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();   // pulsing for afbuigend starts; kopspoor transitions to inRijden
    REQUIRE(f.kopspoor.isInRijden());

    // First cycle completes afbuigend; on reaching idle the wissel sees
    // gewensteRichting=rechtdoor (set by inRijden) and immediately starts a new pulse.
    completeWisselCycle(f);  // afbuigend cycle done → rechtdoor pulsing starts
    completeWisselCycle(f);  // rechtdoor cycle done → richting confirmed
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);
}

// ── inRijden → bezet ──────────────────────────────────────────────────────────

TEST_CASE("kopspoor: sensor bezet during inRijden → bezet") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();  // → inRijden

    f.sensor.setValue(BEZET);  // train arrives at the kopspoor
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::bezet);
}

// ── inRijden → annuleer ───────────────────────────────────────────────────────

TEST_CASE("kopspoor: btnAnnuleer during inRijden → vrij") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.isInRijden());

    f.btnIn.setValue(KNOP_NIET_INGEDUWD);
    f.btnAnnuleer.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
}

// ── bezet → uitRijden ─────────────────────────────────────────────────────────

TEST_CASE("kopspoor: btnUit during bezet + ladder free → uitRijden") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    f.sensor.setValue(BEZET);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::bezet);

    f.btnUit.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::uitRijden);
    REQUIRE(f.relais.getValue() == RELAY_ON);  // track powered for departure
}

TEST_CASE("kopspoor: btnUit during bezet but ladder busy → stays bezet") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    f.sensor.setValue(BEZET);
    f.tick();

    _ksLadderVrij = false;  // another train is on the ladder
    f.btnUit.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::bezet);
}

// ── uitRijden → vrij ──────────────────────────────────────────────────────────

TEST_CASE("kopspoor: sensor clears during uitRijden → vrij") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    f.sensor.setValue(BEZET);
    f.tick();
    f.btnUit.setValue(KNOP_INGEDUWD);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::uitRijden);

    f.sensor.setValue(VRIJ);  // tail of train has cleared
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
    REQUIRE(f.relais.getValue() == RELAY_OFF);  // power cut once track is clear
}

// ── Software triggers ─────────────────────────────────────────────────────────
// triggerIn/Uit/Annuleer mirror the physical buttons — used by the web interface.

TEST_CASE("kopspoor: triggerIn while vrij → inRijden") {
    KopspoorFixture f;
    f.tick();
    f.kopspoor.triggerIn();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::inRijden);
}

TEST_CASE("kopspoor: triggerAnnuleer during inRijden → vrij") {
    KopspoorFixture f;
    f.tick();
    f.kopspoor.triggerIn();
    f.kopspoor.triggerAnnuleer();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
}

TEST_CASE("kopspoor: triggerUit during bezet → uitRijden") {
    KopspoorFixture f;
    f.tick();
    f.kopspoor.triggerIn();
    f.sensor.setValue(BEZET);
    f.tick();
    f.kopspoor.triggerUit(ksLadderVrij);
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::uitRijden);
}
