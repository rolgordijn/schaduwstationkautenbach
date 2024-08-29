#include "Track.h"


Track(int idx)
  : index(idx), spoorStatus(SpoorStatus::initialisatie), state(nullptr), kopSpoorState(nullptr) {}

void Track::setState(TrackState* newState) {
  delete state;
  state = newState;
}

void Track::setState(KopSpoorState* newState) {
  delete kopSpoorState;
  kopSpoorState = newState;
}

void Track::handle() {
  if (state) {
    state->handle(*this);
  } else if (kopSpoorState) {
    kopSpoorState->handle(*this);
  }
}

// Update spoorStatus based on the current state
void Track::updateStatus(SpoorStatus newStatus) {
  spoorStatus = newStatus;
}
