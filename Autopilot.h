#pragma once
#include "IO.h"
#include "Track.h"
#include "constants.h"

// Automatic departure scheduling.
// Sends one train out per VERTREK_INTERVAL_MS; arrivals during the interval are
// queued and fired one by one as time elapses. Switching to manual instantly
// discards the queue so the operator starts with a clean slate.
class Autopilot {
public:
    Autopilot(IO& schakelaar, Track** sporen, int numTracks);
    void init();
    bool isActief() const;

    // Call on every vrij→bezet transition; uitgezondenSpoor = track that just arrived
    // (excluded from departure candidates so it doesn't immediately leave again)
    void treinAangekomen(int uitgezondenSpoor, bool magVertrekken);

    // Call every loop tick for dequeue and queue reset
    void update(bool magVertrekken);

private:
    IO&     schakelaar;
    Track** sporen;
    int     numTracks;

    unsigned long lastVertrekTijd;  // millis() of the last triggered departure
    int           vertrekWachtrij;  // pending departures waiting for the interval

    // Returns a random bezet track index, excluding uitgezondenSpoor.
    // Falls back to uitgezondenSpoor if it is the only occupied track.
    int  vindBezetSpoor(int uitgezondenSpoor) const;
    void triggerVertrek(int uitgezondenSpoor);
};
