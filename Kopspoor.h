#pragma once
#include "IO.h"
#include "Wissel.h"
#include "Knipper.h"
#include "constants.h"

enum class KopspoorStatus : uint8_t {
    vrij,       // empty, ready to receive
    inRijden,   // switches being set, arrival pending — Track LEDs sweep as a visual cue
    bezet,      // train present
    uitRijden   // train departing — relay ON, LED blinking
};

// Dead-end buffer track FSM. btnIn signals the operator's intent; the sensor confirms arrival.
// wissel is the branch point: afbuigend = spoor6 (safe default), rechtdoor = kopspoor.
// The wissel stays afbuigend when vrij so normal trains always reach spoor6 unhindered.
class Kopspoor {
public:
    Kopspoor(IO& sensor, IO& relais, IO& led,
             IO& btnIn, IO& btnAnnuleer,
             IO& btnUit, IO& btnAnnuleerUit,
             Knipper& knipper,
             Wissel& wissel);

    void init();
    // magVertrekken: yard guard — kopspoor will not depart while another track is also departing
    void update(bool (*magVertrekken)());

    KopspoorStatus getStatus()    const;
    bool           isInRijden()   const;
    bool           isUitRijden()  const;
    // Passed to Track::update() during inRijden so all track LEDs animate as a coordinated sweep
    int            getAnimStep()  const;

private:
    IO&      sensor;
    IO&      relais;
    IO&      led;
    IO&      btnIn;
    IO&      btnAnnuleer;
    IO&      btnUit;
    IO&      btnAnnuleerUit;
    Knipper& knipper;
    Wissel&  wissel;

    KopspoorStatus status;
    int            animStep;      // position in the 12-step sweep cycle
    unsigned long  lastAnimTick;
    bool           ledAan;        // kopspoor LED state between sweep cycles

    static const int           ANIM_STEPS   = 12;
    static const unsigned long ANIM_STEP_MS = 150;

    void transitionTo(KopspoorStatus s);
};
