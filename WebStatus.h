#pragma once
#ifdef ARDUINO_UNOR4_WIFI

#include <WiFiS3.h>
#include <stdint.h>

enum class WebCmd : uint8_t {
    none = 0,
    vertrek1, vertrek2, vertrek3, vertrek4, vertrek5, vertrek6,
    kopspoorIn, kopspoorUit, kopspoorAnnuleer, kopspoorAnnuleerUit,
    wisselSet   // index in lastWisselIdx, direction in lastWisselRechtdoor
};

struct YardState {
    uint8_t trackBezet;     // bitmask: bit 0 = spoor 1 … bit 5 = spoor 6
    int     numTracks;
    bool    autoMode;
    bool    entryActief;    // InrijPoort gate traversing
    bool    kanVertrekken;  // ladder free right now
    int     wachtrij;       // queued auto departures
    uint8_t kopspoorStatus; // mirrors KopspoorStatus: 0=vrij 1=inRijden 2=bezet 3=uitRijden
    uint8_t numWissels;     // number of wissels to show (5, or 6 with KOPSPOOR)
    uint8_t wisselRichting; // bitmask: bit N=1 → wissel N+1 is rechtdoor
};

/**
 * @brief Non-blocking status page + button panel for the R4 WiFi.
 *
 * Serves a dark-themed auto-refreshing page on port 80.
 * Buttons mirror the physical panel: departure per bezet track,
 * kopspoor in/uit/annuleer.  Returns the command received this tick
 * so the main sketch can dispatch it; WebStatus itself knows nothing
 * about Track or Kopspoor internals.
 */
class WebStatus {
public:
    WebStatus();
    void begin(const char* ssid, const char* password);

    /** Poll once per loop. Returns WebCmd::none if no client or no command. */
    WebCmd update(const YardState& s);

    // Valid after update() returns wisselSet
    int8_t lastWisselIdx;        // 0-based wissel index (0 = W1 … 5 = W6)
    bool   lastWisselRechtdoor;  // true = rechtdoor, false = afbuigend

private:
    WiFiServer _server;
    WebCmd     _parseCmd(const char* reqLine);
    void       _serveHtml(WiFiClient& client, const YardState& s);
    void       _redirect(WiFiClient& client);
};

#endif
