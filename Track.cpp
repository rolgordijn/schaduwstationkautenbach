#include "Track.h"
#include "IO.h"
#include "Wissel.h"

// Constructor definition
Track::Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, TrackState trackStatus)
  : relay(relay), knop(knop), bezetmelder(bezetmelder), wissel(wissel), led(led), trackStatus(trackStatus) {
  // Additional initialization if needed
}
