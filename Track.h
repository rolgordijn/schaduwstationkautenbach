#pragma once
#include "IO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "constants.h"

enum class TrackStatus {
    initialisatie,
    vrij,
    bezet,
    vertrek,             // relay ON, LED blinking  — phase 1: waiting for front of train at exit sensor
    vertrekGedetecteerd, // relay ON, LED steady     — phase 2: waiting for tail to clear exit sensor
    wisselsRechtdoor     // kopspoor in use: all switches set rechtdoor with a sweeping LED animation
};

// Single staging track: bezetmelder (arrival), relay (track power), wissel (point motor), LED (status).
// The wissel is set afbuigend only when this is the designated entry target; all others stay rechtdoor
// so an arriving train rolls through non-target tracks to reach its assigned track.
class Track {
public:
    Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, Knipper& knipper, int index);

    void init();

    // kopspoorActief / animStep: kopspoor is traversing — override to wisselsRechtdoor with sweep
    // isDoelSpoor: false for non-target tracks when LAATSTE_SPOOR=1 (wissel stays rechtdoor)
    void update(IO& exitSensor, bool magVertrekken, bool kopspoorActief = false, int animStep = 0, bool isDoelSpoor = true);

    TrackStatus getStatus() const;
    void        setStatus(TrackStatus s);

    bool isVrij() const;
    bool isVertrekkend() const; // true for both vertrek and vertrekGedetecteerd

    // Called by the yard for auto-departure mode
    void triggerVertrek();

    // Detect vrij→bezet transitions for auto-departure logic and entry gate
    bool didStatusChange() const;
    void clearStatusChange();

private:
    IO&       relay;
    IO&       knop;
    IO&       bezetmelder;
    Wissel&   wissel;
    IO&       led;
    Knipper&  knipper;
    TrackStatus status;
    int         index;
    bool        statusChanged;

    void transitionTo(TrackStatus newStatus);
    void applyOutputs(bool ledVal, bool relayVal, Richting richting);
};
