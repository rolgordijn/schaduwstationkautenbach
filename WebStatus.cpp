#ifdef ARDUINO_UNOR4_WIFI

#include "WebStatus.h"
#include "debug.h"
#include <string.h>

WebStatus::WebStatus() : _server(80), lastWisselIdx(0), lastWisselRechtdoor(false) {}

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

WebCmd WebStatus::_parseCmd(const char* req) {
    // req is the first HTTP request line: "GET /cmd?v=2 HTTP/1.1"
    if      (strstr(req, "/cmd?v=0"))  return WebCmd::vertrek1;
    else if (strstr(req, "/cmd?v=1"))  return WebCmd::vertrek2;
    else if (strstr(req, "/cmd?v=2"))  return WebCmd::vertrek3;
    else if (strstr(req, "/cmd?v=3"))  return WebCmd::vertrek4;
    else if (strstr(req, "/cmd?v=4"))  return WebCmd::vertrek5;
    else if (strstr(req, "/cmd?v=5"))  return WebCmd::vertrek6;
    else if (strstr(req, "/cmd?ki"))   return WebCmd::kopspoorIn;
    else if (strstr(req, "/cmd?ku"))   return WebCmd::kopspoorUit;
    else if (strstr(req, "/cmd?kau"))  return WebCmd::kopspoorAnnuleerUit;
    else if (strstr(req, "/cmd?ka"))   return WebCmd::kopspoorAnnuleer;
    // wissel: /cmd?w=Nr (rechtdoor) or /cmd?w=Na (afbuigend), N = 0-based index
    const char* wp = strstr(req, "/cmd?w=");
    if (wp && wp[7] >= '0' && wp[7] <= '5' && (wp[8] == 'r' || wp[8] == 'a')) {
        lastWisselIdx       = wp[7] - '0';
        lastWisselRechtdoor = (wp[8] == 'r');
        return WebCmd::wisselSet;
    }
    return WebCmd::none;
}

WebCmd WebStatus::update(const YardState& s) {
    if (WiFi.status() != WL_CONNECTED) return WebCmd::none;
    WiFiClient client = _server.available();
    if (!client) return WebCmd::none;

    unsigned long t = millis();
    while (!client.available() && millis() - t < 500);

    // Read first request line only
    char req[80] = {};
    int len = 0;
    while (client.available() && len < 79) {
        char c = client.read();
        if (c == '\n') break;
        req[len++] = c;
    }
    while (client.available()) client.read();  // drain rest of request

    WebCmd cmd = _parseCmd(req);
    if (cmd != WebCmd::none) {
        _redirect(client);
    } else {
        _serveHtml(client, s);
    }
    client.stop();
    return cmd;
}

void WebStatus::_redirect(WiFiClient& client) {
    client.print(F("HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n"));
}

// ── HTML helpers ──────────────────────────────────────────────────────────────

static void btnOn(WiFiClient& c, const char* href, const char* label) {
    c.print(F("<a href='"));
    c.print(href);
    c.print(F("' class='btn-on'>"));
    c.print(label);
    c.print(F("</a>"));
}

static void btn(WiFiClient& c, const char* href, const char* label, bool warn = false) {
    c.print(F("<a href='"));
    c.print(href);
    c.print(warn ? F("' class='btn-w'>") : F("' class='btn'>"));
    c.print(label);
    c.print(F("</a>"));
}

void WebStatus::_serveHtml(WiFiClient& client, const YardState& s) {
    client.print(F(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<meta http-equiv='refresh' content='2'>"
        "<title>Schaduwstation Kautenbach</title>"
        "<style>"
        "body{font-family:monospace;background:#111;color:#aaa;padding:2rem;max-width:380px;margin:auto}"
        "h2{color:#fff;letter-spacing:.05em;margin-bottom:1.5rem}"
        ".row{display:flex;align-items:center;gap:.6rem;margin:.35rem 0}"
        ".dot{font-size:1.3rem;line-height:1}"
        ".b .dot,.b .lbl{color:#e55}"
        ".v .dot,.v .lbl{color:#4a4}"
        ".lbl{flex:1}"
        ".st{font-size:.78rem;opacity:.6}"
        ".btn{padding:.1rem .5rem;background:#1a3550;color:#6af;border-radius:3px;"
              "text-decoration:none;font-size:.78rem}"
        ".btn:hover{background:#1e4a70}"
        ".btn-w{padding:.1rem .5rem;background:#3a1515;color:#e55;border-radius:3px;"
                "text-decoration:none;font-size:.78rem}"
        ".btn-w:hover{background:#4a1515}"
        ".btn-on{padding:.1rem .5rem;background:#1a3520;color:#4e4;border-radius:3px;"
                "text-decoration:none;font-size:.78rem}"
        ".btn-on:hover{background:#1e4525}"
        ".sep{margin-top:1.2rem;border-top:1px solid #222;padding-top:1rem;font-size:.82rem;line-height:2}"
        ".tag{padding:.1rem .4rem;border-radius:3px;background:#1e1e1e}"
        ".au{color:#6af}.ma{color:#f90}"
        "</style></head><body>"
        "<h2>Schaduwstation Kautenbach</h2>"
    ));

    // ── Track rows ────────────────────────────────────────────────────────────
    for (int i = 0; i < s.numTracks; i++) {
        bool bezet = s.trackBezet & (1 << i);
        client.print(bezet ? F("<div class='row b'>") : F("<div class='row v'>"));
        client.print(bezet ? F("<span class='dot'>&#9679;</span>") : F("<span class='dot'>&#9675;</span>"));
        client.print(F("<span class='lbl'>Spoor "));
        client.print(i + 1);
        client.print(F("</span><span class='st'>"));
        client.print(bezet ? F("bezet") : F("vrij"));
        client.print(F("</span>"));
        if (bezet && s.kanVertrekken) {
            char href[12];
            snprintf(href, sizeof(href), "/cmd?v=%d", i);
            btn(client, href, "vertrek");
        }
        client.print(F("</div>"));
    }

    // ── Meta row ─────────────────────────────────────────────────────────────
    client.print(F("<div class='sep'>"));
    client.print(F("Mode:&nbsp;<span class='tag "));
    client.print(s.autoMode ? F("au'>AUTO") : F("ma'>MANUEEL"));
    client.print(F("</span>"));
    if (s.autoMode && s.wachtrij > 0) {
        client.print(F("&nbsp;&nbsp;wachtrij:&nbsp;"));
        client.print(s.wachtrij);
    }
    client.print(F("<br>Inrijpoort:&nbsp;"));
    client.print(s.entryActief ? F("actief") : F("vrij"));

#if KOPSPOOR == 1
    // kopspoorStatus: 0=vrij 1=inRijden 2=bezet 3=uitRijden
    const char* ksLabels[] = { "vrij", "in rijden", "bezet", "uit rijden" };
    uint8_t ks = s.kopspoorStatus < 4 ? s.kopspoorStatus : 0;
    client.print(F("<br>Kopspoor:&nbsp;"));
    client.print(ksLabels[ks]);
    client.print(F("&nbsp;&nbsp;"));
    switch (ks) {
        case 0: btn(client, "/cmd?ki",  "In");              break;
        case 1: btn(client, "/cmd?ka",  "Annuleer", true);  break;
        case 2: btn(client, "/cmd?ku",  "Uit");             break;
        case 3: btn(client, "/cmd?kau", "Annuleer", true);  break;
    }
#endif

    // ── Wissels ───────────────────────────────────────────────────────────────
    client.print(F("<div class='sep'><b>Wissels</b><br>"));
    for (int i = 0; i < s.numWissels; i++) {
        bool r = s.wisselRichting & (1 << i);
        char hrR[12], hrA[12];
        snprintf(hrR, sizeof(hrR), "/cmd?w=%dr", i);
        snprintf(hrA, sizeof(hrA), "/cmd?w=%da", i);
        client.print(F("<div class='row'><span class='lbl'>W"));
        client.print(i + 1);
        client.print(F("</span><span class='st'>"));
        client.print(r ? F("rechtdoor") : F("afbuigend"));
        client.print(F("</span>&nbsp;"));
        if (r) { btnOn(client, hrR, "R"); btn(client, hrA, "A"); }
        else   { btn(client, hrR, "R");   btnOn(client, hrA, "A"); }
        client.print(F("</div>"));
    }
    client.print(F("</div>"));

    client.print(F("</div></body></html>"));
}

#endif
