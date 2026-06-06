#pragma once
#ifdef ARDUINO_UNOR4_WIFI

#include <WiFiS3.h>
#include <stdint.h>

/**
 * @brief Minimal non-blocking status page for the R4 WiFi.
 *
 * Serves a single auto-refreshing HTML page on port 80 showing track
 * occupancy, mode, and entry gate state. Designed to be polled from a
 * phone or laptop during an operating session without leaving the fascia.
 *
 * Usage:
 *   - Call begin() once in setup() — blocks up to 10 s for WiFi connect.
 *   - Call update() every loop tick — returns immediately if no client waiting.
 */
class WebStatus {
public:
    WebStatus();
    void begin(const char* ssid, const char* password);

    /**
     * @param trackBezet   Bitmask: bit 0 = spoor 1, bit 5 = spoor 6
     * @param numTracks    Total number of regular tracks
     * @param autoMode     true when auto/manual switch is in auto position
     * @param entryActief  true while InrijPoort gate is traversing
     * @param kopspoorVrij true when kopspoor is unoccupied (ignored if !KOPSPOOR)
     * @param wachtrij     Pending auto-departure queue depth
     */
    void update(uint8_t trackBezet, int numTracks, bool autoMode,
                bool entryActief, bool kopspoorVrij, int wachtrij);

private:
    WiFiServer _server;
    void _serve(WiFiClient& client, uint8_t trackBezet, int numTracks,
                bool autoMode, bool entryActief, bool kopspoorVrij, int wachtrij);
};

#endif
