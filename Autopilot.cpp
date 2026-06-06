#include "Autopilot.h"
#include "Arduino.h"
#include "debug.h"

Autopilot::Autopilot(IO& schakelaar, Track** sporen, int numTracks)
    : schakelaar(schakelaar), sporen(sporen), numTracks(numTracks),
      lastVertrekTijd(0), vertrekWachtrij(0) {}

void Autopilot::init() {
    schakelaar.setInput();
}

bool Autopilot::isActief() const {
    return schakelaar.getValue() == KNOP_INGEDUWD;
}

void Autopilot::treinAangekomen(int uitgezondenSpoor, bool magVertrekken) {
    if (!isActief()) return;
    if (magVertrekken && millis() - lastVertrekTijd >= (unsigned long)VERTREK_INTERVAL_MS) {
        triggerVertrek(uitgezondenSpoor);
    } else {
        vertrekWachtrij++;
        debugln(F("auto: vertrek in wachtrij gezet"));
    }
}

void Autopilot::update(bool magVertrekken) {
    if (!isActief()) {
        vertrekWachtrij = 0;
        return;
    }
    if (vertrekWachtrij > 0 &&
            magVertrekken &&
            millis() - lastVertrekTijd >= (unsigned long)VERTREK_INTERVAL_MS) {
        // Pick any bezet track (no exclusion — no specific arrival to protect here)
        int matchIndices[6];
        int matchCount = 0;
        for (int i = 0; i < numTracks; i++) {
            if (sporen[i]->getStatus() == TrackStatus::bezet) matchIndices[matchCount++] = i;
        }
        if (matchCount > 0) {
            sporen[matchIndices[random(matchCount)]]->triggerVertrek();
            lastVertrekTijd = millis();
            debugln(F("auto: vertrek uit wachtrij"));
        }
        vertrekWachtrij--;  // always consume — no bezet tracks means this slot is dropped
    }
}

int Autopilot::vindBezetSpoor(int uitgezondenSpoor) const {
    int matchIndices[6];
    int matchCount = 0;
    for (int i = 0; i < numTracks; i++) {
        if (i != uitgezondenSpoor && sporen[i]->getStatus() == TrackStatus::bezet)
            matchIndices[matchCount++] = i;
    }
    if (matchCount > 0) return matchIndices[random(matchCount)];
    return uitgezondenSpoor;
}

void Autopilot::triggerVertrek(int uitgezondenSpoor) {
    sporen[vindBezetSpoor(uitgezondenSpoor)]->triggerVertrek();
    lastVertrekTijd = millis();
}
