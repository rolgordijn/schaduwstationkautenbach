#ifdef ARDUINO_UNOR4_WIFI

#include "WebStatus.h"
#include "debug.h"

WebStatus::WebStatus() : _server(80) {}

void WebStatus::begin(const char* ssid, const char* password) {
    debugln(F("wifi: verbinden..."));
    WiFi.begin(ssid, password);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) delay(200);
    if (WiFi.status() != WL_CONNECTED) { debugln(F("wifi: geen verbinding")); return; }
    _server.begin();
    { char b[32]; snprintf(b, sizeof(b), "wifi: %d.%d.%d.%d",
        WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      debugln(b); }
}

void WebStatus::update(uint8_t trackBezet, int numTracks, bool autoMode,
                       bool entryActief, bool kopspoorVrij, int wachtrij) {
    if (WiFi.status() != WL_CONNECTED) return;
    WiFiClient client = _server.available();
    if (!client) return;
    unsigned long t = millis();
    while (!client.available() && millis() - t < 500);
    while (client.available()) client.read();  // drain the HTTP request
    _serve(client, trackBezet, numTracks, autoMode, entryActief, kopspoorVrij, wachtrij);
    client.stop();
}

void WebStatus::_serve(WiFiClient& client, uint8_t trackBezet, int numTracks,
                       bool autoMode, bool entryActief, bool kopspoorVrij, int wachtrij) {
    client.print(F(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<meta http-equiv='refresh' content='2'>"
        "<title>Schaduwstation Kautenbach</title>"
        "<style>"
        "body{font-family:monospace;background:#111;color:#aaa;padding:2rem;max-width:360px;margin:auto}"
        "h2{color:#fff;letter-spacing:.05em;margin-bottom:1.5rem}"
        ".row{display:flex;align-items:center;gap:.7rem;margin:.35rem 0}"
        ".dot{font-size:1.3rem;line-height:1}"
        ".b .dot,.b .name{color:#e55}"
        ".v .dot,.v .name{color:#4a4}"
        ".name{flex:1}"
        ".status{font-size:.8rem;opacity:.7}"
        ".meta{margin-top:1.4rem;border-top:1px solid #2a2a2a;padding-top:1rem;font-size:.85rem;line-height:1.8}"
        ".tag{padding:.1rem .45rem;border-radius:3px;background:#1e1e1e}"
        ".auto{color:#6af}.man{color:#f90}"
        "</style></head><body>"
        "<h2>Schaduwstation Kautenbach</h2>"
    ));

    for (int i = 0; i < numTracks; i++) {
        bool bezet = trackBezet & (1 << i);
        client.print(F("<div class='row "));
        client.print(bezet ? F("b'>") : F("v'>"));
        client.print(F("<span class='dot'>"));
        client.print(bezet ? F("&#9679;") : F("&#9675;"));
        client.print(F("</span><span class='name'>Spoor "));
        client.print(i + 1);
        client.print(F("</span><span class='status'>"));
        client.print(bezet ? F("bezet") : F("vrij"));
        client.print(F("</span></div>"));
    }

    client.print(F("<div class='meta'>"));
    client.print(F("Mode:&nbsp;<span class='tag "));
    client.print(autoMode ? F("auto'>AUTO") : F("man'>MANUEEL"));
    client.print(F("</span>"));
    if (autoMode && wachtrij > 0) {
        client.print(F(" &nbsp; wachtrij: "));
        client.print(wachtrij);
    }
    client.print(F("<br>Inrijpoort:&nbsp;"));
    client.print(entryActief ? F("actief") : F("vrij"));
#if KOPSPOOR == 1
    client.print(F("<br>Kopspoor:&nbsp;"));
    client.print(kopspoorVrij ? F("vrij") : F("bezet"));
#endif
    client.print(F("</div></body></html>"));
}

#endif
