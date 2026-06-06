#include <catch2/catch_test_macros.hpp>
#include "Arduino.h"
#include "VirtualIO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "Kopspoor.h"
#include "constants.h"

static bool _ksLadderVrij = true;
static bool ksLadderVrij() { return _ksLadderVrij; }

struct KopspoorFixture {
    VirtualIO sensor, relais, led;
    VirtualIO btnIn, btnAnnuleer, btnUit, btnAnnuleerUit;
    Knipper   knipper;
    VirtualIO wisselR, wisselA;
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
        Wissel::resetCDU();
        Wissel::setIsLadderVrijFn(ksLadderVrij);
        kopspoor.init();
    }

    void tick() {
        wissel.update();
        kopspoor.update(ksLadderVrij);
    }
};

static void completeWisselCycle(KopspoorFixture& f) {
    advanceTime(200);  // > PULS_MS (125 ms)
    f.tick();          // pulsing ends, settling starts
    advanceTime(50);   // > SETTLING_MS (25 ms)
    f.tick();          // settling ends, richting confirmed
}

// ── Initial state ─────────────────────────────────────────────────────────────

TEST_CASE("kopspoor: initial status is vrij") {
    KopspoorFixture f;
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
}

TEST_CASE("kopspoor: wissel defaults to afbuigend (spoor6 accessible)") {
    KopspoorFixture f;
    f.tick();  // vrij: activate(afbuigend) queued
    f.tick();  // pulsing starts (CDU acquired)
    completeWisselCycle(f);  // pulse + settling → richting confirmed
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
    f.tick();   // pulsing for afbuigend starts; kopspoor → inRijden (activate(rechtdoor) queued)
    REQUIRE(f.kopspoor.isInRijden());
    completeWisselCycle(f);  // afbuigend cycle done; idle sees rechtdoor → pulsing starts
    completeWisselCycle(f);  // rechtdoor cycle done
    REQUIRE(f.wissel.getRichting() == Richting::rechtdoor);
}

// ── inRijden → bezet ──────────────────────────────────────────────────────────

TEST_CASE("kopspoor: sensor bezet during inRijden → bezet") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();

    f.sensor.setValue(BEZET);
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
    REQUIRE(f.relais.getValue() == RELAY_ON);
}

TEST_CASE("kopspoor: btnUit during bezet but ladder busy → stays bezet") {
    KopspoorFixture f;
    f.tick();
    f.btnIn.setValue(KNOP_INGEDUWD);
    f.tick();
    f.sensor.setValue(BEZET);
    f.tick();

    _ksLadderVrij = false;
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

    f.sensor.setValue(VRIJ);
    f.tick();
    REQUIRE(f.kopspoor.getStatus() == KopspoorStatus::vrij);
    REQUIRE(f.relais.getValue() == RELAY_OFF);
}

// ── Software triggers ─────────────────────────────────────────────────────────

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
