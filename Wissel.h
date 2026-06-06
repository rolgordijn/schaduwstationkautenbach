#pragma once
#include "IO.h"

enum class Richting : uint8_t {
    onbekend  = 0,
    rechtdoor = 1,
    afbuigend = 2
};

enum class WisselState : uint8_t {
    idle,
    pulsing,   // control pin high
    settling   // control pin low — settling time before declaring done
};

class Wissel {
public:
    Wissel(IO* rechtdoor, IO* afbuigend, int id);
    Wissel();

    void init();

    // Record desired direction — non-blocking, safe to call every loop regardless of ladder state
    void activate(Richting r);
    // Register the yard's ladder-free query — called once at init, queried inside update()
    static void setIsLadderVrijFn(bool (*fn)());

    // Advance the FSM — must be called every loop iteration for all wissels
    // Only one wissel may pulse at a time (static semaphore enforced here)
    void update();

    void zetafbuigend();
    void zetrechtdoor();
    void off();

    Richting getRichting() const;
    bool     isBusy() const; // true while pulse or settle is in progress

private:
    IO*           pinRechtdoor;
    IO*           pinAfbuigend;
    Richting      richting;         // confirmed physical position
    Richting      gewensteRichting; // requested direction
    Richting      activeRichting;   // direction of the pulse currently in progress
    WisselState   state;
    unsigned long stateStart;
    int           id;

    // Semaphore: only the wissel pointed to here may pulse
    static Wissel* actief;
    // Queried at the idle→pulsing transition; nullptr = always allow (safe default)
    static bool (*isLadderVrijFn)();
};
