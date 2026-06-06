# Schaduwstation Kautenbach

> *Schaduwstation* (Dutch) — a hidden staging yard beneath a model railway layout, where trains wait their turn out of sight of the audience.

An Arduino controller that fully automates the staging yard of the Kautenbach layout: routing arriving trains to free tracks, holding back queued arrivals, protecting turnouts from operating under load, and dispatching trains on a configurable schedule — all without blocking the main loop for a single millisecond.

---

## The problem it solves

A staging yard is invisible to visitors but operationally brutal. Six tracks fan out from a single entry throat through five CDU-fired turnouts. Trains arrive one at a time, need to be held at the entry section while the previous one clears the point ladder, routed to the right track, and eventually dispatched — either manually or automatically at timed intervals. Do any of that with `delay()` and you get jitter, missed sensor edges, and points that fire while a train is still rolling over them.

This firmware handles all of it with non-blocking finite state machines, one per object.

---

## Layout

```
                        ┌── spoor 1
                   W1 ──┤
                        └── W2 ──┬── spoor 2
  Entry                          └── W3 ──┬── spoor 3
  sensor ──► [InrijPoort] ────────────────└── W4 ──┬── spoor 4
                                                    └── W5 ──┬── spoor 5
                                                             └──── spoor 6
                                                                      │
                                                                   W6 ┤  (KOPSPOOR build only)
                                                                      └── kopspoor ◄ buffer track
                        Exit sensor (3 m past throat)
```

- **5 turnouts (W1–W5)** fan the entry throat into **6 regular berths**
- An optional **6th turnout (W6)** branches the last berth into a **dead-end buffer track (kopspoor)**
- The **entry relay** (InrijPoort) serialises arrivals at the throat
- The **exit sensor** sits 3 m past the yard exit and prevents a departing track from being released too early

---

## Features

| | |
|---|---|
| **Non-blocking FSMs** | Every object (Track, Wissel, Kopspoor, InrijPoort, Autopilot) runs its own state machine; `loop()` never calls `delay()` |
| **CDU protection** | A static semaphore ensures only one turnout motor fires at a time; the ladder-free guard (`magVertrekken`) blocks new pulses while any train is on the throat |
| **Entry gate sequencing** | Phase 1: relay ON for `INRIJ_VERTRAGING_MS` to let the loco clear the relay-controlled rail gap. Phase 2: relay OFF while a second train waits at the entry sensor |
| **Chained arrivals** | If a train is waiting at the entry section when the gate closes, it detects the falling `yardVol` edge and restarts immediately for that train |
| **Automatic departure scheduling** | In auto mode, one train departs per `VERTREK_INTERVAL_MS` (default 60 s). Arrivals during the interval are queued; the queue drains automatically and is discarded on manual mode switch |
| **Dead-end (kopspoor) protection** | W6 defaults to *afbuigend* (→ spoor 6); switches to *rechtdoor* (→ kopspoor) only during the operator-initiated arrival sequence. Kopspoor exit uses a separate physical route — W6 is not involved |
| **Configurable build** | `KOPSPOOR`, `LAATSTE_SPOOR`, and `DEBUG` are compile-time flags in `constants.h` |
| **Timestamped debug output** | Every `debugln()` call is prefixed with `[millis]` over Serial at 115200 baud |

---

## Hardware

| Component | Detail |
|---|---|
| **MCU** | Arduino (ATmega328P or compatible) |
| **I/O expanders** | 3× MCP23017 via I²C (`0x27`, `0x20`, `0x21`) |
| **Turnout motors** | CDU-fired solenoids on mcp0 pins 0–11 (pairs: even = *afbuigend*, odd = *rechtdoor*) |
| **Track relays** | Arduino pins 10–13, A1–A3 (⚠ A2 shares with analog current sense) |
| **Occupancy sensors** | Active-low on mcp1 pins 8–15 and mcp2 pins 0–3 |
| **Status LEDs** | Arduino pins 6–9 (tracks 1–4), mcp0 pins 12–15, mcp2 pins 8–15 |
| **Buttons** | mcp1 pins 0–7, mcp2 pins 4–7 (active-low, internal pull-up) |

Full pin mapping is in [`hardware_map.h`](hardware_map.h). To rewire, change only that file.

---

## Software architecture

```
schaduwstation_kopie_1703.ino   — wiring: global objects, setup(), loop()
│
├── Track          — per-berth FSM (vrij / bezet / vertrek / vertrekGedetecteerd / wisselsRechtdoor)
├── Wissel         — CDU pulse FSM with static semaphore; queries magVertrekken() via function pointer
├── InrijPoort     — entry gate (forceAan phase → houdTegen phase → chain detection)
├── Kopspoor       — dead-end buffer track FSM; owns W6 and drives it directly
├── Autopilot      — auto departure scheduler (interval timer + queue + manual-mode reset)
│
├── IO / BasicIO / MCP23017IO   — hardware abstraction, change-detection, edge-detection
├── Knipper        — non-blocking blinker
├── StopWatch      — elapsed time helper
└── debug.h        — conditional Serial output with [millis] timestamp prefix
```

### Key design decisions

**Function pointer for ladder guard** — `Wissel` holds a `static bool (*isLadderVrijFn)()` set once at `setup()`. The idle→pulsing transition queries it on demand. This avoids pushing state into Wissel every loop and keeps the dependency injected rather than hardcoded.

**`wisselNoop`** — spoor 6 is at the far end of the 5-point ladder; no physical point controls access to it. It uses a null-pin `Wissel` instance whose `activate()` confirms the direction immediately without going near the CDU semaphore.

**InrijPoort chain detection** — the gate uses a `lastYardVol` latch. When a departure completes while a second train is still at the entry sensor, `lastYardVol` falls from `true` to `false` with `inrijBezet` still high, triggering an immediate gate restart for that waiting train.

---

## Configuration

Edit `constants.h`:

```cpp
#define KOPSPOOR        1       // 1 = build includes dead-end buffer track + W6
#define LAATSTE_SPOOR   0       // 0 = route to first free track, 1 = last free track
#define DEBUG           1       // 1 = Serial debug output at 115200 baud

#define INRIJ_VERTRAGING_MS   3000    // relay-on time after entry detection
#define INRIJ_TIMEOUT_MS     30000    // safety fallback if no track arrival is detected
#define VERTREK_INTERVAL_MS  60000    // minimum gap between automatic departures
```

---

## Building & uploading

Open `schaduwstation_kopie_1703.ino` in the Arduino IDE (or CLI), select your board, and upload. No external libraries beyond the MCP23017 driver are required.

```bash
arduino-cli compile --fqbn arduino:avr:uno schaduwstation_kopie_1703.ino
arduino-cli upload  --fqbn arduino:avr:uno --port /dev/ttyUSB0 schaduwstation_kopie_1703.ino
```

---

## Operating the yard

### Automatic mode
Flip `schakelaarAutoManueel` to auto. The yard dispatches one train per minute after each arrival. If a train arrives while the interval is still running, its departure is queued and fires as soon as the next slot opens. Switching back to manual instantly clears the queue.

### Manual mode
Press the departure button (`btnVertrek1`–`6`) for the track you want to send. The departure is blocked while the ladder is occupied (another train arriving or departing); button presses during that window are ignored, not queued.

### Kopspoor *(KOPSPOOR build only)*
1. Press `btnKopspoorIn` — W6 switches to *rechtdoor*, track LEDs sweep as a visual cue to confirm all points are set
2. Drive the train in; sensor confirms arrival
3. Press `btnKopspoorUit` when ready to dispatch — relay powers the dead-end track, train exits via its own independent exit route
4. `btnAnnuleer` / `btnAnnuleerKopspoorUit` abort the respective operation at any point

---

## License

Do whatever you like with this. If it saves you from a runaway consist disappearing off the end of a staging track at an exhibition, we call that a win.
