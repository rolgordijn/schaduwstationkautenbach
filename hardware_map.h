#pragma once

// ═══════════════════════════════════════════════════════════════════════════════
//  hardware_map.h  —  PCB label → functional name
//
//  To rewire: change the right-hand side of the relevant #define.
//  The rest of the code never needs to change.
//
//  Physical connections
//  ────────────────────
//  knop1–6          mcp1 pins 0–5     departure buttons, tracks 1–6
//  knop7–8          mcp1 pins 6–7     yard buttons
//  knop9–12         mcp2 pins 4–7     yard buttons (continued)
//
//  bezetmelder1–6   mcp1 pins 8–13    track occupancy sensors 1–6
//  bezetmelder7–8   mcp1 pins 14–15   kopspoor + entry sensors
//  bezetmelder9–12  mcp2 pins 0–3     exit sensor + reserve
//
//  relay1–6         Arduino 10,11,12,13,A3,A2   track relays  (⚠ relay6/A2 conflicts with analog)
//  relay7           Arduino A1                  kopspoor relay
//  relay8           Arduino 2                   entry relay
//
//  led1–4           Arduino 9,8,7,6             track LEDs 1–4
//  led5–6           mcp0 pins 12–13             track LEDs 5–6
//  led7–8           mcp0 pins 14–15             kopspoor + entry LEDs
//  led9–16          mcp2 pins 8–15              exit LED + reserve
//
//  wissel1–6        mcp0 pins 0–11 (pairs rechtdoor/afbuigend: 1/0, 3/2, …, 11/10)
// ═══════════════════════════════════════════════════════════════════════════════

// ── Departure buttons ─────────────────────────────────────────────────────────
#define btnVertrek1  knop1
#define btnVertrek2  knop2
#define btnVertrek3  knop3
#define btnVertrek4  knop4
#define btnVertrek5  knop5
#define btnVertrek6  knop6

// ── Yard control buttons ──────────────────────────────────────────────────────
#define btnKopspoorIn          knop7
#define btnReserve             knop8
#define btnAnnuleer            knop9   // cancel kopspoor-in, or confirm arrival
#define btnKopspoorUit         knop10
#define btnAnnuleerKopspoorUit knop11
#define schakelaarAutoManueel  knop12

// ── Track occupancy sensors ───────────────────────────────────────────────────
#define sensorSpoor1  bezetmelder1
#define sensorSpoor2  bezetmelder2
#define sensorSpoor3  bezetmelder3
#define sensorSpoor4  bezetmelder4
#define sensorSpoor5  bezetmelder5
#define sensorSpoor6  bezetmelder6

// ── Yard sensors ──────────────────────────────────────────────────────────────
#define sensorKopspoor  bezetmelder7
#define sensorInrij     bezetmelder8
#define sensorUitrij    bezetmelder9   // 3 m past yard — prevents premature track release

// ── Reserve sensors ───────────────────────────────────────────────────────────
#define sensorReserve10  bezetmelder10
#define sensorReserve11  bezetmelder11
#define sensorReserve12  bezetmelder12

// ── Track relays ──────────────────────────────────────────────────────────────
#define relaisSpoor1  relay1
#define relaisSpoor2  relay2
#define relaisSpoor3  relay3
#define relaisSpoor4  relay4
#define relaisSpoor5  relay5
#define relaisSpoor6  relay6

// ── Yard relays ───────────────────────────────────────────────────────────────
#define relaisKopspoor  relay7
#define relaisInrij     relay8

// ── Track status LEDs ─────────────────────────────────────────────────────────
#define ledSpoor1  led1
#define ledSpoor2  led2
#define ledSpoor3  led3
#define ledSpoor4  led4
#define ledSpoor5  led5
#define ledSpoor6  led6

// ── Yard LEDs ─────────────────────────────────────────────────────────────────
#define ledKopspoor  led7
#define ledInrij     led8
#define ledUitrij    led9

// ── Reserve LEDs ─────────────────────────────────────────────────────────────
#define ledReserve10  led10
#define ledReserve11  led11
#define ledReserve12  led12
#define ledReserve13  led13
#define ledReserve14  led14
#define ledReserve15  led15
#define ledReserve16  led16
