#pragma once
#ifdef ARDUINO_UNOR4_WIFI

#include <WiFiS3.h>
#include <stdint.h>

enum class WebCmd : uint8_t {
    none = 0,
    vertrek1, vertrek2, vertrek3, vertrek4, vertrek5, vertrek6,
    kopspoorIn, kopspoorUit, kopspoorAnnuleer, kopspoorAnnuleerUit
};

struct YardState {
    uint8_t trackBezet;     // bitmask: bit 0 = spoor 1 … bit 5 = spoor 6
    int     numTracks;
    bool    autoMode;
    bool    entryActief;    // InrijPoort gate traversing
    bool    kanVertrekken;  // ladder free right now
    int     wachtrij;       // queued auto departures
    uint8_t kopspoorStatus; // mirrors KopspoorStatus: 0=vrij 1=inRijden 2=bezet 3=uitRijden
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

private:
    WiFiServer _server;
    WebCmd     _parseCmd(const char* reqLine);
    void       _serveHtml(WiFiClient& client, const YardState& s);
    void       _redirect(WiFiClient& client);
};

#endif
