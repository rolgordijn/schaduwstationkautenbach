#include "InrijPoort.h"
#include "Arduino.h"
#include "debug.h"

InrijPoort::InrijPoort(IO& sensor, IO& relais, IO& led,
                       IO& sensorUit, IO& ledUit,
                       Knipper& knipper)
    : sensor(sensor), relais(relais), led(led),
      sensorUit(sensorUit), ledUit(ledUit), knipper(knipper),
      gateActief(false), gateStart(0),
      inrijBezet(false), inrijVorige(false), lastYardVol(false) {}

void InrijPoort::init() {
    relais.init(OUTPUT, LOW);
    led.init(OUTPUT, LOW);
    ledUit.init(OUTPUT, LOW);
    sensor.setInput();
    sensorUit.setInput();
}

void InrijPoort::update(bool yardVol, bool kopspoorInRijden) {
    inrijBezet    = (sensor.getValue() == BEZET);
    bool stijgend = inrijBezet && !inrijVorige;  // rising edge: new train entered entry section
    inrijVorige   = inrijBezet;

    // Gate starts when ladder is free AND:
    //   a) new rising edge (train just entered), OR
    //   b) departure just cleared (yardVol fell) while a train was already waiting at the entry section
    // The second case handles a train that rolled in during a departure and was held by the OFF relay.
    bool wachtendTreinVrijgegeven = inrijBezet && lastYardVol;  // departure just cleared with train waiting
    bool gateGestart = !gateActief && !yardVol && (stijgend || wachtendTreinVrijgegeven);
    if (gateGestart) { gateActief = true; gateStart = millis(); }

    lastYardVol = yardVol;

    unsigned long looptijd = gateActief ? millis() - gateStart : 0;

    if (gateActief && looptijd >= (unsigned long)INRIJ_TIMEOUT_MS) {
        gateActief = false;
        debugln(F("inrijpoort: timeout, gate geforceerd gesloten"));
    }

    // forceAan:  relay stays ON while the loco moves clear of the relay-controlled section
    // houdTegen: relay OFF once it's safe to cut — holds the next train at the entry section
    bool forceAan  = gateActief && looptijd < (unsigned long)INRIJ_VERTRAGING_MS;
    bool houdTegen = gateActief && looptijd >= (unsigned long)INRIJ_VERTRAGING_MS && inrijBezet;

    bool relayAan;
    if      (forceAan)  relayAan = true;
    else if (houdTegen) relayAan = false;
    else                relayAan = !yardVol || kopspoorInRijden;

    relais.setValue(relayAan ? RELAY_ON : RELAY_OFF);
    led.setValue(inrijBezet ? (relayAan ? LED_ON : knipper.getValue()) : LED_OFF);
    ledUit.setValue(sensorUit.getValue() == BEZET ? LED_ON : LED_OFF);

#if DEBUG == 1
    if (gateGestart)                            debugln(wachtendTreinVrijgegeven ? F("inrijpoort: ladder vrij, wachtende trein vrijgegeven") : F("inrijpoort: trein gedetecteerd, gate start"));
    static bool lastForceAan = false;
    if (!lastForceAan && forceAan)              debugln(F("inrijpoort: vertraging actief"));
    if (lastForceAan && !forceAan && houdTegen) debugln(F("inrijpoort: blokkering actief, trein wacht"));
    lastForceAan = forceAan;
    static bool lastRelay = false;
    if (relayAan != lastRelay) {
        debugln(relayAan ? F("inrijpoort: relais open") : F("inrijpoort: relais gesloten"));
        lastRelay = relayAan;
    }
#endif
}

bool InrijPoort::isTraversing() const { return gateActief; }

void InrijPoort::treinAangekomen() {
    if (!gateActief) return;
    if (millis() - gateStart < (unsigned long)INRIJ_VERTRAGING_MS) return;
    gateActief = false;
    debugln(F("inrijpoort: trein aangekomen, gate sluit"));
    if (inrijBezet) {
        // A second train was held on the entry section; restart the gate for it immediately
        gateActief = true;
        gateStart  = millis();
        debugln(F("inrijpoort: volgende trein wacht, gate herstart"));
    }
}
