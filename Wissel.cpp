#include "Wissel.h"
#include "Arduino.h"
#include "debug.h"

// Only one wissel may pulse at a time — nullptr means the channel is free
Wissel*  Wissel::actief         = nullptr;
bool   (*Wissel::isLadderVrijFn)() = nullptr;

void Wissel::setIsLadderVrijFn(bool (*fn)()) { isLadderVrijFn = fn; }

static const unsigned long PULS_MS     = 125;
static const unsigned long SETTLING_MS = 25;

Wissel::Wissel()
    : pinRechtdoor(nullptr), pinAfbuigend(nullptr),
      richting(Richting::onbekend), gewensteRichting(Richting::onbekend),
      activeRichting(Richting::onbekend),
      state(WisselState::idle), stateStart(0), id(0) {}

Wissel::Wissel(IO* rechtdoor, IO* afbuigend, int id)
    : pinRechtdoor(rechtdoor), pinAfbuigend(afbuigend),
      richting(Richting::onbekend), gewensteRichting(Richting::onbekend),
      activeRichting(Richting::onbekend),
      state(WisselState::idle), stateStart(0), id(id) {}

void Wissel::init() {
    pinRechtdoor->init(OUTPUT, LOW);
    pinAfbuigend->init(OUTPUT, LOW);
}

void Wissel::activate(Richting r) {
    // No physical pins means this is a dummy wissel (e.g. the last track in the ladder
    // that is reached when all real points are straight) — confirm instantly, no CDU pulse.
    if (!pinRechtdoor) { richting = r; return; }
    gewensteRichting = r;
}

void Wissel::update() {
    switch (state) {

        case WisselState::idle:
            // CDU (capacitor-discharge unit) needs time to recharge — simultaneous pulses would be weak or damaging
            // isLadderVrijFn: halt here only — pulsing/settling states are never interrupted
            if ((!isLadderVrijFn || isLadderVrijFn())
                    && actief == nullptr
                    && gewensteRichting != Richting::onbekend
                    && gewensteRichting != richting) {
                actief         = this;
                activeRichting = gewensteRichting; // snapshot direction at pulse start
                state          = WisselState::pulsing;
                stateStart     = millis();
                IO* pin = (activeRichting == Richting::rechtdoor) ? pinRechtdoor : pinAfbuigend;
                pin->setHigh();
                { char b[28]; snprintf(b, sizeof(b), "wissel %d: %s", id, activeRichting == Richting::rechtdoor ? "rechtdoor" : "afbuigend"); debugln(b); }
            }
            break;

        case WisselState::pulsing:
            if (millis() - stateStart >= PULS_MS) {
                IO* pin = (activeRichting == Richting::rechtdoor) ? pinRechtdoor : pinAfbuigend;
                pin->setLow();
                state      = WisselState::settling;
                stateStart = millis();
            }
            break;

        case WisselState::settling:
            if (millis() - stateStart >= SETTLING_MS) {
                // Confirm the physical position that was actually pulsed.
                // If gewensteRichting changed mid-pulse, the idle check will
                // queue another pulse on the next available cycle.
                richting = activeRichting;
                state    = WisselState::idle;
                actief   = nullptr;
            }
            break;
    }
}

void Wissel::zetafbuigend() { activate(Richting::afbuigend); }
void Wissel::zetrechtdoor() { activate(Richting::rechtdoor); }

Richting Wissel::getRichting() const { return richting; }
bool     Wissel::isBusy()     const  { return state != WisselState::idle; }

void Wissel::off() {
    if (pinRechtdoor) pinRechtdoor->setLow();
    if (pinAfbuigend) pinAfbuigend->setLow();
}
