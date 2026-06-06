#include "Track.h"
#include "Arduino.h"
#include "debug.h"

Track::Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, Knipper& knipper, int index)
    : relay(relay), knop(knop), bezetmelder(bezetmelder),
      wissel(wissel), led(led), knipper(knipper),
      status(TrackStatus::initialisatie), index(index), statusChanged(false) {}

void Track::init() {
    led.init(OUTPUT, LOW);
    relay.init(OUTPUT, LOW);
    knop.setInput();
    bezetmelder.setInput();
    status = TrackStatus::initialisatie;
    statusChanged = false;
}

void Track::transitionTo(TrackStatus newStatus) {
    if (status != newStatus) {
        status = newStatus;
        statusChanged = true;
    }
}

void Track::applyOutputs(bool ledVal, bool relayVal, Richting richting) {
    led.setValue(ledVal);
    relay.setValue(relayVal);
    wissel.activate(richting);
}

TrackStatus Track::getStatus() const       { return status; }
void        Track::setStatus(TrackStatus s) { transitionTo(s); }
bool        Track::isVrij() const          { return status == TrackStatus::vrij; }
bool        Track::isVertrekkend() const   {
    return status == TrackStatus::vertrek || status == TrackStatus::vertrekGedetecteerd;
}
void Track::triggerVertrek() {
    if (status == TrackStatus::bezet) transitionTo(TrackStatus::vertrek);
}
bool Track::didStatusChange() const { return statusChanged; }
void Track::clearStatusChange()     { statusChanged = false; }

void Track::update(IO& exitSensor, bool magVertrekken, bool kopspoorActief, int animStep, bool isDoelSpoor) {
    switch (status) {

        case TrackStatus::initialisatie:
            // Sync wissel to the physical state found at boot — avoids a spurious pulse on the first update
            if (bezetmelder.getValue() == BEZET) {
                transitionTo(TrackStatus::bezet);
                wissel.zetrechtdoor();
            } else {
                transitionTo(TrackStatus::vrij);
                wissel.zetafbuigend();
            }
            break;

        case TrackStatus::vrij:
#if LAATSTE_SPOOR == 1
            // Non-target tracks stay rechtdoor so the arriving train passes through to the target
            applyOutputs(LED_OFF, RELAY_OFF, isDoelSpoor ? Richting::afbuigend : Richting::rechtdoor);
#else
            applyOutputs(LED_OFF, RELAY_OFF, Richting::afbuigend);
#endif
            if (kopspoorActief) {
                transitionTo(TrackStatus::wisselsRechtdoor);
            } else if (bezetmelder.getValue() == BEZET) {
                transitionTo(TrackStatus::bezet);
                { char b[20]; snprintf(b, sizeof(b), "spoor %d: bezet", index + 1); debugln(b); }
            }
            break;

        case TrackStatus::bezet:
            applyOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor);
            if (kopspoorActief) {
                transitionTo(TrackStatus::wisselsRechtdoor);
            } else if (knop.getValue() == KNOP_INGEDUWD && magVertrekken) {
                transitionTo(TrackStatus::vertrek);
                { char b[24]; snprintf(b, sizeof(b), "spoor %d: vertrekken", index + 1); debugln(b); }
            }
            break;

        case TrackStatus::vertrek:
            // Phase 1: relay ON + LED blinks until the front of the train reaches the exit sensor
            applyOutputs(knipper.getValue(), RELAY_ON, Richting::rechtdoor);
            if (exitSensor.getValue() == BEZET) {
                transitionTo(TrackStatus::vertrekGedetecteerd);
            }
            break;

        case TrackStatus::vertrekGedetecteerd:
            // Phase 2: LED steady while the tail clears — cutting relay power early would stall the last wagons
            applyOutputs(LED_ON, RELAY_ON, Richting::rechtdoor);
            if (exitSensor.getValue() == VRIJ) {
                transitionTo(TrackStatus::vrij);
                { char b[18]; snprintf(b, sizeof(b), "spoor %d: vrij", index + 1); debugln(b); }
            }
            break;

        case TrackStatus::wisselsRechtdoor: {
            // animStep ticks through 2×NUM_TRACKS steps: even = this track's LED ON, odd = OFF
            bool ledOn = (animStep / 2 == index) && (animStep % 2 == 0);
            led.setValue(ledOn);
            relay.setValue(RELAY_OFF);
            wissel.activate(Richting::rechtdoor);
            if (!kopspoorActief) {
                transitionTo(TrackStatus::initialisatie);
            }
            break;
        }
    }
}
