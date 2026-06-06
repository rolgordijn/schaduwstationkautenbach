#pragma once
#include "IO.h"
#include "Knipper.h"
#include "constants.h"

// Serialises entry: holds waiting trains at the entry section while the first crosses the point ladder.
// Phase 1 (forceAan): relay forced ON for INRIJ_VERTRAGING_MS so the loco clears the relay-controlled section.
// Phase 2 (houdTegen): relay OFF while sensor is still BEZET — waiting train is held in place.
// Gate closes when treinAangekomen() is called (train arrived at its track); if sensor is still BEZET
// a second train is waiting and the gate immediately restarts for it.
class InrijPoort {
public:
    InrijPoort(IO& sensor, IO& relais, IO& led,
               IO& sensorUit, IO& ledUit,
               Knipper& knipper);

    void init();
    void update(bool yardVol, bool kopspoorInRijden);
    // Call once per loop when any track bezetmelder fires a vrij→bezet transition
    void treinAangekomen();
    // True while a train is anywhere between entry section and its assigned track
    bool isTraversing() const;

private:
    IO&      sensor;
    IO&      relais;
    IO&      led;
    IO&      sensorUit;
    IO&      ledUit;
    Knipper& knipper;

    bool          gateActief;   // true from first entry BEZET until treinAangekomen() confirms arrival
    unsigned long gateStart;    // millis() snapshot used to time the vertraging and timeout phases
    bool          inrijBezet;   // sensor state cached in update(), read by treinAangekomen() without extra I/O
    bool          inrijVorige;  // previous-frame sensor value for rising-edge detection
    bool          lastYardVol;  // previous-frame yardVol for detecting when departure clears
};
