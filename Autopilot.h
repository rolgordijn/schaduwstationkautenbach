#pragma once
#include "IO.h"
#include "Track.h"
#include "constants.h"

/**
 * @brief Automatic departure scheduling for the staging yard.
 *
 * Fires one departure per @ref VERTREK_INTERVAL_MS. When a train arrives
 * while the interval has not yet elapsed, the departure is queued and
 * dispatched as soon as the interval expires. Switching the hardware toggle
 * to manual mode instantly drains the queue so the operator starts clean.
 *
 * Usage:
 *   - Call init() once in setup().
 *   - Call treinAangekomen() on every vrij→bezet track transition.
 *   - Call update() every loop tick.
 */
class Autopilot {
public:
    Autopilot(IO& schakelaar, Track** sporen, int numTracks);

    /** Configure the auto/manual switch pin — call once in setup(). */
    void init();

    /** @return true while the hardware toggle is in auto position. */
    bool isActief() const;

    /** @return number of pending departures waiting for the interval to elapse. */
    int getWachtrij() const;

    /**
     * @brief Respond to a new arrival.
     *
     * @param uitgezondenSpoor  Index of the track that just turned bezet;
     *                          excluded from departure candidates so the
     *                          fresh arrival does not immediately leave.
     * @param magVertrekken     Current ladder-free guard result.
     *
     * If the interval has elapsed and the ladder is free, a departure fires
     * immediately; otherwise the slot is added to the queue.
     */
    void treinAangekomen(int uitgezondenSpoor, bool magVertrekken);

    /**
     * @brief Dequeue one pending departure when conditions allow.
     *
     * Also resets the queue when not in auto mode, so a mode switch always
     * leaves a clean state.
     *
     * @param magVertrekken  Current ladder-free guard result.
     */
    void update(bool magVertrekken);

private:
    IO&     schakelaar;
    Track** sporen;
    int     numTracks;

    unsigned long lastVertrekTijd;  // millis() of the last triggered departure
    int           vertrekWachtrij;  // pending departures waiting for the interval

    /**
     * @brief Pick a random bezet track, excluding uitgezondenSpoor.
     * @return uitgezondenSpoor if it is the only occupied track (fallback).
     */
    int  vindBezetSpoor(int uitgezondenSpoor) const;
    void triggerVertrek(int uitgezondenSpoor);
};
