#define ALL_INPUT  0xFF
#define ALL_OUTPUT 0x00

#define KOPSPOOR      1
#define LAATSTE_SPOOR 0   // 0 = eerste vrije spoor, 1 = laatste vrije spoor
#define DEBUG         1
#define BOARD_VERSIE  2

#define BEZET 1
#define VRIJ  0

// Entry gate: holds a second arriving train while the first traverses the point ladder.
// INRIJ_VERTRAGING_MS — relay stays ON after entry BEZET; lets first train's loco clear
//                        the relay-controlled section before power can be cut.
// INRIJ_TIMEOUT_MS    — safety fallback: gate closes after this long even if no track
//                        arrival is detected (bezetmelder failure protection).
#define INRIJ_VERTRAGING_MS 3000
#define INRIJ_TIMEOUT_MS    30000

// Minimum time between two automatic departures in auto mode.
// Arrivals during this window are queued and fired one by one as the interval elapses.
#define VERTREK_INTERVAL_MS 60000

#define KNOP_INGEDUWD      0
#define KNOP_NIET_INGEDUWD 1

#define LED_OFF 0
#define LED_ON  1

#define RELAY_ON  1
#define RELAY_OFF 0
