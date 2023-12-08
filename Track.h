#ifndef TRACK_H
#define TRACK_H

class IO;
class Wissel;

// Forward declaration of TrackState enum class
enum class TrackState;

class Track {
public:
  // Constructor
  Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, TrackState trackStatus);

private:
  IO& relay;
  IO& knop;
  IO& bezetmelder;
  Wissel& wissel;
  IO& led;
  TrackState trackStatus;
};

#endif  // TRACK_H
#ifndef TRACK_H
#define TRACK_H

class IO;
class Wissel;

// Forward declaration of TrackState enum class
enum class TrackState;

class Track {
public:
  // Constructor
  Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, TrackState trackStatus);

private:
  IO& relay;
  IO& knop;
  IO& bezetmelder;
  Wissel& wissel;
  IO& led;
  TrackState trackStatus;
};

#endif  // TRACK_H
