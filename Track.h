#pragma once 


class Track {
public:
    int index;
    SpoorStatus spoorStatus;
    TrackState* state;
    KopSpoorState* kopSpoorState;
    Track(int idx) : index(idx), spoorStatus(SpoorStatus::initialisatie), state(nullptr), kopSpoorState(nullptr) {}
    void setState(TrackState* newState);
    void setState(KopSpoorState* newState);
    void handle() ;
    // Update spoorStatus based on the current state
    void updateStatus(SpoorStatus newStatus);
};
