#include "IO.h"
#include "Track.h"
#include "Wissel.h"
#include "Kopspoor.h"
#include "InrijPoort.h"
#include "Autopilot.h"
#include "debug.h"
#include "knipper.h"
#include "constants.h"
#include "hardware_map.h"
#include "StopWatch.h"
#ifdef ARDUINO_UNOR4_WIFI
#include "WebStatus.h"
#include "secrets.h"
WebStatus webStatus;
#else
#include <avr/wdt.h>
#endif

#pragma GCC optimize("-O")

// ── MCP23017 I/O expanders ────────────────────────────────────────────────────

MCP23017 mcp0 = MCP23017(0x27);
MCP23017 mcp1 = MCP23017(0x20);
MCP23017 mcp2 = MCP23017(0x21);

// ── Wissel control pins (mcp0 pins 0–11; even = afbuigend, odd = rechtdoor) ───

MCP23017IO wafb1(&mcp0,  0);  MCP23017IO wrec1(&mcp0,  1);
MCP23017IO wafb2(&mcp0,  2);  MCP23017IO wrec2(&mcp0,  3);
MCP23017IO wafb3(&mcp0,  4);  MCP23017IO wrec3(&mcp0,  5);
MCP23017IO wafb4(&mcp0,  6);  MCP23017IO wrec4(&mcp0,  7);
MCP23017IO wafb5(&mcp0,  8);  MCP23017IO wrec5(&mcp0,  9);
#if KOPSPOOR == 1
MCP23017IO wafb6(&mcp0, 10);  MCP23017IO wrec6(&mcp0, 11);
#endif

// ── Point motors ──────────────────────────────────────────────────────────────

Wissel wissel1(&wrec1, &wafb1, 1);
Wissel wissel2(&wrec2, &wafb2, 2);
Wissel wissel3(&wrec3, &wafb3, 3);
Wissel wissel4(&wrec4, &wafb4, 4);
Wissel wissel5(&wrec5, &wafb5, 5);
#if KOPSPOOR == 1
Wissel wissel6(&wrec6, &wafb6, 6);
#endif
// spoor6 is the far end of the 5-point ladder — no dedicated physical point.
// wisselNoop satisfies Track's Wissel& reference; activate() confirms the
// direction immediately without queuing a CDU pulse.
Wissel wisselNoop;

// ── Track status LEDs ─────────────────────────────────────────────────────────

BasicIO    led1  = BasicIO(9,  OUTPUT);       // ledSpoor1
BasicIO    led2  = BasicIO(8,  OUTPUT);       // ledSpoor2
BasicIO    led3  = BasicIO(7,  OUTPUT);       // ledSpoor3
BasicIO    led4  = BasicIO(6,  OUTPUT);       // ledSpoor4
MCP23017IO led5  = MCP23017IO(&mcp0, 12);    // ledSpoor5
MCP23017IO led6  = MCP23017IO(&mcp0, 13);    // ledSpoor6
MCP23017IO led7  = MCP23017IO(&mcp0, 14);    // ledKopspoor
MCP23017IO led8  = MCP23017IO(&mcp0, 15);    // ledInrij
MCP23017IO led9  = MCP23017IO(&mcp2,  8);    // ledUitrij
MCP23017IO led10 = MCP23017IO(&mcp2,  9);    // ledReserve10
MCP23017IO led11 = MCP23017IO(&mcp2, 10);    // ledReserve11
MCP23017IO led12 = MCP23017IO(&mcp2, 11);    // ledReserve12
MCP23017IO led13 = MCP23017IO(&mcp2, 12);    // ledReserve13
MCP23017IO led14 = MCP23017IO(&mcp2, 13);    // ledReserve14
MCP23017IO led15 = MCP23017IO(&mcp2, 14);    // ledReserve15
MCP23017IO led16 = MCP23017IO(&mcp2, 15);    // ledReserve16

// ── Buttons ───────────────────────────────────────────────────────────────────

MCP23017IO knop1  = MCP23017IO(&mcp1, 0);    // btnVertrek1
MCP23017IO knop2  = MCP23017IO(&mcp1, 1);    // btnVertrek2
MCP23017IO knop3  = MCP23017IO(&mcp1, 2);    // btnVertrek3
MCP23017IO knop4  = MCP23017IO(&mcp1, 3);    // btnVertrek4
MCP23017IO knop5  = MCP23017IO(&mcp1, 4);    // btnVertrek5
MCP23017IO knop6  = MCP23017IO(&mcp1, 5);    // btnVertrek6
MCP23017IO knop7  = MCP23017IO(&mcp1, 6);    // btnKopspoorIn
MCP23017IO knop8  = MCP23017IO(&mcp1, 7);    // btnReserve
MCP23017IO knop9  = MCP23017IO(&mcp2, 4);    // btnAnnuleer
MCP23017IO knop10 = MCP23017IO(&mcp2, 5);    // btnKopspoorUit
MCP23017IO knop11 = MCP23017IO(&mcp2, 6);    // btnAnnuleerKopspoorUit
MCP23017IO knop12 = MCP23017IO(&mcp2, 7);    // schakelaarAutoManueel

// ── Occupancy sensors ─────────────────────────────────────────────────────────

MCP23017IO bezetmelder1  = MCP23017IO(&mcp1,  8);   // sensorSpoor1
MCP23017IO bezetmelder2  = MCP23017IO(&mcp1,  9);   // sensorSpoor2
MCP23017IO bezetmelder3  = MCP23017IO(&mcp1, 10);   // sensorSpoor3
MCP23017IO bezetmelder4  = MCP23017IO(&mcp1, 11);   // sensorSpoor4
MCP23017IO bezetmelder5  = MCP23017IO(&mcp1, 12);   // sensorSpoor5
MCP23017IO bezetmelder6  = MCP23017IO(&mcp1, 13);   // sensorSpoor6
MCP23017IO bezetmelder7  = MCP23017IO(&mcp1, 14);   // sensorKopspoor
MCP23017IO bezetmelder8  = MCP23017IO(&mcp1, 15);   // sensorInrij
MCP23017IO bezetmelder9  = MCP23017IO(&mcp2,  0);   // sensorUitrij
MCP23017IO bezetmelder10 = MCP23017IO(&mcp2,  1);   // sensorReserve10
MCP23017IO bezetmelder11 = MCP23017IO(&mcp2,  2);   // sensorReserve11
MCP23017IO bezetmelder12 = MCP23017IO(&mcp2,  3);   // sensorReserve12

// ── Relays ────────────────────────────────────────────────────────────────────

BasicIO relay1 = BasicIO(10, OUTPUT);   // relaisSpoor1
BasicIO relay2 = BasicIO(11, OUTPUT);   // relaisSpoor2
BasicIO relay3 = BasicIO(12, OUTPUT);   // relaisSpoor3
BasicIO relay4 = BasicIO(13, OUTPUT);   // relaisSpoor4
BasicIO relay5 = BasicIO(A3, OUTPUT);   // relaisSpoor5
BasicIO relay6 = BasicIO(A2, OUTPUT);   // relaisSpoor6  ⚠ A2 conflicts with analog current sense
BasicIO relay7 = BasicIO(A1, OUTPUT);   // relaisKopspoor
BasicIO relay8 = BasicIO(2,  OUTPUT);   // relaisInrij

// ── Shared utilities ──────────────────────────────────────────────────────────

Knipper   knipper     = Knipper(100, 500);
Knipper   knippersnel = Knipper(50,  100);
StopWatch stopWatch   = StopWatch();
BasicIO   buzzer      = BasicIO(4, OUTPUT);

// ── Track and wissel arrays ────────────────────────────────────────────────────
// n points → n+1 regular berths: 5 points always give 6 regular tracks.
// wissel6 (kopspoor branch) is owned by the Kopspoor object, not this array.

const int NUM_TRACKS = 6;
Wissel* wissels[5] = { &wissel1, &wissel2, &wissel3, &wissel4, &wissel5 };

// ── Track (spoor) objects — constructor args use functional names from hardware_map.h ──

Track spoor1(relaisSpoor1, btnVertrek1, sensorSpoor1, wissel1, ledSpoor1, knipper, 0);
Track spoor2(relaisSpoor2, btnVertrek2, sensorSpoor2, wissel2, ledSpoor2, knipper, 1);
Track spoor3(relaisSpoor3, btnVertrek3, sensorSpoor3, wissel3, ledSpoor3, knipper, 2);
Track spoor4(relaisSpoor4, btnVertrek4, sensorSpoor4, wissel4, ledSpoor4, knipper, 3);
Track spoor5(relaisSpoor5, btnVertrek5, sensorSpoor5, wissel5, ledSpoor5, knipper, 4);
Track spoor6(relaisSpoor6, btnVertrek6, sensorSpoor6, wisselNoop, ledSpoor6, knipper, 5);

Track* sporen[6] = { &spoor1, &spoor2, &spoor3, &spoor4, &spoor5, &spoor6 };

// LED array — used only for the startup sweep animation
IO* alleLeds[16] = {
    &led1,  &led2,  &led3,  &led4,  &led5,  &led6,
    &led7,  &led8,  &led9,  &led10, &led11, &led12,
    &led13, &led14, &led15, &led16
};

// ── Kopspoor ──────────────────────────────────────────────────────────────────

#if KOPSPOOR == 1
Kopspoor kopspoor(sensorKopspoor, relaisKopspoor, ledKopspoor,
                  btnKopspoorIn, btnAnnuleer,
                  btnKopspoorUit, btnAnnuleerKopspoorUit,
                  knipper, wissel6);
#endif

// ── InrijPoort ────────────────────────────────────────────────────────────────

InrijPoort inrijpoort(sensorInrij, relaisInrij, ledInrij,
                      sensorUitrij, ledUitrij,
                      knipper);

// ── Autopilot ─────────────────────────────────────────────────────────────────

Autopilot autopilot(schakelaarAutoManueel, sporen, NUM_TRACKS);

// ── Helpers ───────────────────────────────────────────────────────────────────

void mcp23017Reset(MCP23017& mcp, int IPOL_A, int IPOL_B) {
    debugln(F("reset mcp23017"));
    mcp.init();
    mcp.writeRegister(MCP23017Register::IPOL_A, IPOL_A);
    mcp.writeRegister(MCP23017Register::IPOL_B, IPOL_B);
    mcp.writeRegister(MCP23017Register::GPPU_A, 0);
    mcp.writeRegister(MCP23017Register::GPPU_B, 0);
    mcp.writeRegister(MCP23017Register::GPIO_A, 0);
    mcp.writeRegister(MCP23017Register::GPIO_B, 0);
}

int aantalSporenBezet() {
    // Any active departure treats the yard as full — closes the entry gate while a train is on the ladder
    for (int i = 0; i < NUM_TRACKS; i++) {
        if (sporen[i]->isVertrekkend()) return NUM_TRACKS;
    }
    int n = 0;
    for (int i = 0; i < NUM_TRACKS; i++) {
        if (!sporen[i]->isVrij()) n++;
    }
    return n;
}

bool magVertrekken() {
    // Point ladder is shared: block departure while any train is on it (departing, arriving, or kopspoor)
    for (int i = 0; i < NUM_TRACKS; i++) {
        if (sporen[i]->isVertrekkend()) return false;
    }
    if (inrijpoort.isTraversing()) return false;
#if KOPSPOOR == 1
    if (kopspoor.isInRijden() || kopspoor.isUitRijden()) return false;
#endif
    return true;
}

void debugSensors() {
#if DEBUG == 1
    IO* sensors[12] = {
        &sensorSpoor1,   &sensorSpoor2,   &sensorSpoor3,   &sensorSpoor4,
        &sensorSpoor5,   &sensorSpoor6,   &sensorKopspoor, &sensorInrij,
        &sensorUitrij,   &sensorReserve10, &sensorReserve11, &sensorReserve12
    };
    const char* labels[12] = {
        "sensorSpoor1", "sensorSpoor2", "sensorSpoor3",  "sensorSpoor4",
        "sensorSpoor5", "sensorSpoor6", "sensorKopspoor","sensorInrij",
        "sensorUitrij", "sensorReserve10","sensorReserve11","sensorReserve12"
    };
    for (int i = 0; i < 12; i++) {
        if (sensors[i]->didChange()) {
            char buf[32]; snprintf(buf, sizeof(buf), "%s: %s", labels[i], sensors[i]->getValue() ? "bezet" : "vrij");
            debugln(buf);
            sensors[i]->clearChangedFlag();
        }
    }
#endif
}

void debugRelais() {
#if DEBUG == 1
    IO* relais[8] = {
        &relaisSpoor1, &relaisSpoor2, &relaisSpoor3, &relaisSpoor4,
        &relaisSpoor5, &relaisSpoor6, &relaisKopspoor, &relaisInrij
    };
    const char* labels[8] = {
        "relaisSpoor1","relaisSpoor2","relaisSpoor3","relaisSpoor4",
        "relaisSpoor5","relaisSpoor6","relaisKopspoor","relaisInrij"
    };
    for (int i = 0; i < 8; i++) {
        if (relais[i]->didChange()) {
            char buf[32]; snprintf(buf, sizeof(buf), "%s: %s", labels[i], relais[i]->getValue() ? "aan" : "uit");
            debugln(buf);
            relais[i]->clearChangedFlag();
        }
    }
#endif
}

void debugKnoppen() {
#if DEBUG == 1
    IO* knoppen[12] = {
        &btnVertrek1, &btnVertrek2, &btnVertrek3,    &btnVertrek4,
        &btnVertrek5, &btnVertrek6, &btnKopspoorIn,  &btnReserve,
        &btnAnnuleer, &btnKopspoorUit, &btnAnnuleerKopspoorUit, &schakelaarAutoManueel
    };
    const char* labels[12] = {
        "btnVertrek1","btnVertrek2","btnVertrek3",   "btnVertrek4",
        "btnVertrek5","btnVertrek6","btnKopspoorIn", "btnReserve",
        "btnAnnuleer","btnKopspoorUit","btnAnnuleerKopspoorUit","schakelaarAutoManueel"
    };
    for (int i = 0; i < 12; i++) {
        if (knoppen[i]->didChange() && knoppen[i]->getValue() == KNOP_INGEDUWD) {
            debugln(labels[i]);
            knoppen[i]->clearChangedFlag();
        }
    }
#endif
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    debugln(F("start init"));
    Wire.begin();

    mcp23017Reset(mcp0, 0, 0);
    mcp23017Reset(mcp1, 0, 0);
    mcp23017Reset(mcp2, 0, 0);
    delay(100);

    debugln(F("init wissels"));
    for (int i = 0; i < NUM_TRACKS - 1; i++) wissels[i]->init();

    debugln(F("init sporen"));
    for (int i = 0; i < NUM_TRACKS; i++) sporen[i]->init();

    // Yard-level IOs not owned by Track, Kopspoor or InrijPoort objects
#if KOPSPOOR == 1
    kopspoor.init();
#endif
    inrijpoort.init();
    for (int i = 9; i < 16; i++) alleLeds[i]->init(OUTPUT, LOW);

    Wissel::setIsLadderVrijFn(magVertrekken);
    autopilot.init();
    btnReserve.setInput();

    sensorReserve10.setInput();
    sensorReserve11.setInput();
    sensorReserve12.setInput();

    debugln(F("LED sweep"));
    for (IO* led : alleLeds) { led->setValue(HIGH); delay(100); }
    for (IO* led : alleLeds) led->setValue(LOW);

    pinMode(A0, INPUT);
#ifdef ARDUINO_UNOR4_WIFI
    webStatus.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
    debugln(F("EINDE setup"));
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
    debugRelais();
    debugSensors();
    debugKnoppen();

#if KOPSPOOR == 1
    kopspoor.update(magVertrekken);
#endif

    // ── Entry gate + relay + LEDs ─────────────────────────────────────────────
    bool yardVol = (aantalSporenBezet() >= NUM_TRACKS);
#if KOPSPOOR == 1
    inrijpoort.update(yardVol, kopspoor.isInRijden());
#else
    inrijpoort.update(yardVol, false);
#endif

    // ── Point motors ─────────────────────────────────────────────────────────
    bool kanVertrekken = magVertrekken();
    for (int i = 0; i < NUM_TRACKS - 1; i++) wissels[i]->update();

    // ── Wissel test mode: hold btnReserve, press a departure button to toggle ─
    {
        static bool prevKnop[6] = {};
        bool testModus = (btnReserve.getValue() == KNOP_INGEDUWD);
        IO* knopWissel[6] = { &btnVertrek1, &btnVertrek2, &btnVertrek3,
                               &btnVertrek4, &btnVertrek5, &btnVertrek6 };
        for (int i = 0; i < 6; i++) {
            bool cur = (knopWissel[i]->getValue() == KNOP_INGEDUWD);
            if (testModus && cur && !prevKnop[i]) {
                if (i < 5) {
                    Richting huidig = wissels[i]->getRichting();
                    wissels[i]->activate(huidig == Richting::rechtdoor ? Richting::afbuigend : Richting::rechtdoor);
                }
#if KOPSPOOR == 1
                else {
                    Richting huidig = wissel6.getRichting();
                    wissel6.activate(huidig == Richting::rechtdoor ? Richting::afbuigend : Richting::rechtdoor);
                }
#endif
            }
            prevKnop[i] = cur;
        }
    }

    // ── Per-track state machine ───────────────────────────────────────────────

#if LAATSTE_SPOOR == 1
    // Last free track in ascending order — its wissel will be set afbuigend, all others rechtdoor
    int doelSpoor = -1;
    for (int i = NUM_TRACKS - 1; i >= 0; i--) {
        if (sporen[i]->isVrij()) { doelSpoor = i; break; }
    }
#endif

    for (int i = 0; i < NUM_TRACKS; i++) {
#if LAATSTE_SPOOR == 1
        bool isDoelSpoor = (i == doelSpoor);
#else
        bool isDoelSpoor = true;
#endif
#if KOPSPOOR == 1
        sporen[i]->update(sensorUitrij, kanVertrekken, kopspoor.isInRijden(), kopspoor.getAnimStep(), isDoelSpoor);
#else
        sporen[i]->update(sensorUitrij, kanVertrekken, false, 0, isDoelSpoor);
#endif

        if (sporen[i]->didStatusChange() && sporen[i]->getStatus() == TrackStatus::bezet) {
            inrijpoort.treinAangekomen();
            autopilot.treinAangekomen(i, magVertrekken());
        }

        sporen[i]->clearStatusChange();
    }

    // ── Test mode: LED test + auto-reset on exit ──────────────────────────────
    {
        static bool prevTestModus = false;
        bool testModus = (btnReserve.getValue() == KNOP_INGEDUWD);
        if (testModus) {
            IO* leds[6]  = { &ledSpoor1, &ledSpoor2, &ledSpoor3,
                              &ledSpoor4, &ledSpoor5, &ledSpoor6 };
            IO* knops[6] = { &btnVertrek1, &btnVertrek2, &btnVertrek3,
                              &btnVertrek4, &btnVertrek5, &btnVertrek6 };
            for (int i = 0; i < 6; i++)
                leds[i]->setValue(knops[i]->getValue() == KNOP_INGEDUWD ? LED_ON : LED_OFF);
        } else if (prevTestModus) {
            // Falling edge of btnReserve: leaving test mode → hardware reset
#ifdef ARDUINO_UNOR4_WIFI
            NVIC_SystemReset();
#else
            wdt_enable(WDTO_15MS);
            while (1) {}
#endif
        }
        prevTestModus = testModus;
    }

    // ── Autopilot dequeue ─────────────────────────────────────────────────────
    autopilot.update(magVertrekken());

#ifdef ARDUINO_UNOR4_WIFI
    {
        uint8_t bezet = 0;
        for (int i = 0; i < NUM_TRACKS; i++)
            if (!sporen[i]->isVrij()) bezet |= (1 << i);
        uint8_t wRichting = 0;
        for (int i = 0; i < 5; i++)
            if (wissels[i]->getRichting() == Richting::rechtdoor) wRichting |= (1 << i);
#if KOPSPOOR == 1
        if (wissel6.getRichting() == Richting::rechtdoor) wRichting |= (1 << 5);
#endif
        YardState ys = {
            bezet, NUM_TRACKS,
            autopilot.isActief(),
            inrijpoort.isTraversing(),
            kanVertrekken,
            autopilot.getWachtrij(),
#if KOPSPOOR == 1
            (uint8_t)kopspoor.getStatus(),
#else
            0,
#endif
            (uint8_t)(KOPSPOOR == 1 ? 6 : 5),
            wRichting
        };
        switch (webStatus.update(ys)) {
            case WebCmd::vertrek1: if (kanVertrekken) sporen[0]->triggerVertrek(); break;
            case WebCmd::vertrek2: if (kanVertrekken) sporen[1]->triggerVertrek(); break;
            case WebCmd::vertrek3: if (kanVertrekken) sporen[2]->triggerVertrek(); break;
            case WebCmd::vertrek4: if (kanVertrekken) sporen[3]->triggerVertrek(); break;
            case WebCmd::vertrek5: if (kanVertrekken) sporen[4]->triggerVertrek(); break;
            case WebCmd::vertrek6: if (kanVertrekken) sporen[5]->triggerVertrek(); break;
#if KOPSPOOR == 1
            case WebCmd::kopspoorIn:          kopspoor.triggerIn();            break;
            case WebCmd::kopspoorUit:         kopspoor.triggerUit(magVertrekken); break;
            case WebCmd::kopspoorAnnuleer:    kopspoor.triggerAnnuleer();      break;
            case WebCmd::kopspoorAnnuleerUit: kopspoor.triggerAnnuleerUit();   break;
#endif
            case WebCmd::wisselSet: {
                int8_t idx = webStatus.lastWisselIdx;
                Richting r = webStatus.lastWisselRechtdoor ? Richting::rechtdoor : Richting::afbuigend;
                if (idx >= 0 && idx < 5) wissels[idx]->activate(r);
#if KOPSPOOR == 1
                else if (idx == 5) wissel6.activate(r);
#endif
                break;
            }
            default: break;
        }
    }
#endif
}
