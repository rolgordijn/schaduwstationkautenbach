class TrackState {
public:
    virtual ~TrackState() {}
    virtual void handle(Track& track) = 0;
};


class VrijState : public TrackState {
public:
  void handle(Track& track) override;
};

class BezetState : public TrackState {
public:
  void handle(Track& track) override;
};



class VertrekState : public TrackState {
public:
  void handle(Track& track) override;
};


class WisselsRechtdoorState : public TrackState {
public:
  void WisselsRechtdoorState::handle(Track& track) override;
};

// Implement other state classes (VertrekState, WisselsRechtdoorState, etc.)
