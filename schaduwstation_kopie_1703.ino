#include "IO.h"
#include "Wissel.h"
#include "debug.h"
#include "knipper.h"
#include "debug.h"

#include "constants.h"


#include "StopWatch.h"

//#pragma GCC optimize("-O")

//wisselsrechtdoor, enkel bij kopspoor gebruikt.
enum class SpoorStatus { initialisatie,
                         vrij,
                         bezet,
                         vertrek,
                         wisselsRechtdoor
};

struct Track {  // Structure declaration
  long lastDepartureTimestamp;
  SpoorStatus state;
};

long globalLastDepartureTime;


Track tracks[6];

#if KOPSPOOR == 1

enum class KopSpoorStatus { init,
                            vrij,
                            bezet,
                            in,
                            uit
};
KopSpoorStatus kopSpoorStatus = KopSpoorStatus::init;

#endif


const int analogInputPin = A2;      // Analog input pin for current measurement
const int peakCurrentResetPin = 2;  // Pin for the reset pulse
unsigned long resetTime = 100;      // Reset pulse duration in milliseconds

const int buzzerPin = 4;


BasicIO buzzer = BasicIO(buzzerPin, OUTPUT);


MCP23017 mcp2 = MCP23017(0x21);
MCP23017 mcp1 = MCP23017(0x20);
MCP23017 mcp0 = MCP23017(0x27);

Wissel wissel1 = Wissel(new MCP23017IO(&mcp0, 1), new MCP23017IO(&mcp0, 0), 1);
Wissel wissel2 = Wissel(new MCP23017IO(&mcp0, 3), new MCP23017IO(&mcp0, 2), 2);
Wissel wissel3 = Wissel(new MCP23017IO(&mcp0, 5), new MCP23017IO(&mcp0, 4), 3);
Wissel wissel4 = Wissel(new MCP23017IO(&mcp0, 7), new MCP23017IO(&mcp0, 6), 4);
Wissel wissel5 = Wissel(new MCP23017IO(&mcp0, 9), new MCP23017IO(&mcp0, 8), 5);

#if KOPSPOOR == 1
Wissel wissel6 = Wissel(new MCP23017IO(&mcp0, 11), new MCP23017IO(&mcp0, 10), 6);
#endif

BasicIO led1 = BasicIO(9, OUTPUT);
BasicIO led2 = BasicIO(8, OUTPUT);
BasicIO led3 = BasicIO(7, OUTPUT);
BasicIO led4 = BasicIO(6, OUTPUT);

MCP23017IO led5 = MCP23017IO(&mcp0, 12);
MCP23017IO led6 = MCP23017IO(&mcp0, 13);
MCP23017IO led7 = MCP23017IO(&mcp0, 14);
MCP23017IO led8 = MCP23017IO(&mcp0, 15);

MCP23017IO led9 = MCP23017IO(&mcp2, 8);
MCP23017IO led10 = MCP23017IO(&mcp2, 9);
MCP23017IO led11 = MCP23017IO(&mcp2, 10);
MCP23017IO led12 = MCP23017IO(&mcp2, 11);
MCP23017IO led13 = MCP23017IO(&mcp2, 12);
MCP23017IO led14 = MCP23017IO(&mcp2, 13);
MCP23017IO led15 = MCP23017IO(&mcp2, 14);
MCP23017IO led16 = MCP23017IO(&mcp2, 15);

MCP23017IO knop1 = MCP23017IO(&mcp1, 0);
MCP23017IO knop2 = MCP23017IO(&mcp1, 1);
MCP23017IO knop3 = MCP23017IO(&mcp1, 2);
MCP23017IO knop4 = MCP23017IO(&mcp1, 3);
MCP23017IO knop5 = MCP23017IO(&mcp1, 4);
MCP23017IO knop6 = MCP23017IO(&mcp1, 5);
MCP23017IO knop7 = MCP23017IO(&mcp1, 6);
MCP23017IO knop8 = MCP23017IO(&mcp1, 7);

MCP23017IO bezetmelder9 = MCP23017IO(&mcp2, 0);
MCP23017IO bezetmelder10 = MCP23017IO(&mcp2, 1);
MCP23017IO bezetmelder11 = MCP23017IO(&mcp2, 2);
MCP23017IO bezetmelder12 = MCP23017IO(&mcp2, 3);
MCP23017IO knop9 = MCP23017IO(&mcp2, 4);
MCP23017IO knop10 = MCP23017IO(&mcp2, 5);
MCP23017IO knop11 = MCP23017IO(&mcp2, 6);
MCP23017IO knop12 = MCP23017IO(&mcp2, 7);

MCP23017IO bezetmelder1 = MCP23017IO(&mcp1, 8);
MCP23017IO bezetmelder2 = MCP23017IO(&mcp1, 9);
MCP23017IO bezetmelder3 = MCP23017IO(&mcp1, 10);
MCP23017IO bezetmelder4 = MCP23017IO(&mcp1, 11);
MCP23017IO bezetmelder5 = MCP23017IO(&mcp1, 12);
MCP23017IO bezetmelder6 = MCP23017IO(&mcp1, 13);
MCP23017IO bezetmelder7 = MCP23017IO(&mcp1, 14);
MCP23017IO bezetmelder8 = MCP23017IO(&mcp1, 15);

BasicIO relay1 = BasicIO(10, OUTPUT);
BasicIO relay2 = BasicIO(11, OUTPUT);
BasicIO relay3 = BasicIO(12, OUTPUT);
BasicIO relay4 = BasicIO(13, OUTPUT);
BasicIO relay5 = BasicIO(A3, OUTPUT);
BasicIO relay6 = BasicIO(A2, OUTPUT);
BasicIO relay7 = BasicIO(A1, OUTPUT);
BasicIO relay8 = BasicIO(2, OUTPUT);

BasicIO auxSwitch = BasicIO(5, INPUT);


StopWatch stopWatch = StopWatch();
Knipper knipper = Knipper(100, 500);

Knipper knippersnel = Knipper(50, 100);

#if KOPSPOOR == 1
Wissel* wissels[] = { &wissel1, &wissel2, &wissel3, &wissel4, &wissel5, &wissel6 };
#else
Wissel* wissels[] = { &wissel1, &wissel2, &wissel3, &wissel4, &wissel5 };
#endif


IO* relays[] = { &relay1, &relay2, &relay3, &relay4, &relay5, &relay6, &relay7, &relay8 };
IO* knoppen[] = { &knop1, &knop2, &knop3, &knop4, &knop5, &knop6, &knop7, &knop8, &knop9, &knop10, &knop11, &knop12 };
IO* bezetmelders[] = { &bezetmelder1, &bezetmelder2, &bezetmelder3, &bezetmelder4, &bezetmelder5, &bezetmelder6, &bezetmelder7, &bezetmelder8, &bezetmelder9, &bezetmelder10, &bezetmelder11, &bezetmelder12 };
IO* leds[] = { &led1, &led2, &led3, &led4, &led5, &led6, &led7, &led8, &led9, &led10, &led11, &led12, &led13, &led14, &led15, &led16 };


bool trainLeftInLastNSeconds(unsigned long n) {
  unsigned long currentTime = millis();

  for (int i = 0; i < 6; ++i) {
    unsigned long timeSinceLastDeparture = currentTime - tracks[i].lastDepartureTimestamp;

    if (timeSinceLastDeparture <= (n * 1000)) {  // Convert seconds to milliseconds
      return true;                               // At least one track had a departure in the last N seconds
    }
  }

  return false;  // No track had a departure in the last N seconds
}



bool isCurrentAboveThreshold() {
  int rawValue = analogRead(A2);              // Read the raw ADC value from A2
  int millivolts = (rawValue * 5000) / 1023;  // Convert raw value to millivolts (assuming 10-bit ADC and 5V reference)
  int milliamps = millivolts * 2;             // Calculate current in milliamperes based on the 0.5 ohm sense resistor

  // Check if current is above the threshold
  bool aboveThreshold = (milliamps > 1500);

  // Reset peak value
  digitalWrite(peakCurrentResetPin, HIGH);  // Apply the reset pulse
  delay(resetTime);
  digitalWrite(peakCurrentResetPin, LOW);

  return aboveThreshold;
}


void debugBezetmelders() {
#if DEBUG == 1
  for (int i = 0; i < 8; i++) {
    if (bezetmelders[i]->didChange()) {
      debug("bezetmelder ");
      debug(i + 1);

      if (bezetmelders[i]->getValue()) {
        debugln(" bezet ");
      } else {
        debugln(" vrij ");
      }
      bezetmelders[i]->clearChangedFlag();
    }
  }
#endif
}


int aantalSporenMetStatus(SpoorStatus status) {
  int n = 0;
  for (Track track : tracks) {
    if (track.state == status) n++;
  }
  return n;
}


int magVertrekken() {
  bool geenVertrekkendeTreinen = aantalSporenMetStatus(SpoorStatus::vertrek) == 0;
  return geenVertrekkendeTreinen && kopSpoorStatus != KopSpoorStatus::uit;
}

void debugRelays() {
#if DEBUG == 1
  for (int i = 0; i < 8; i++) {
    if (relays[i]->didChange()) {
      debug("relay ");
      debug(i + 1);

      if (relays[i]->getValue()) {
        debugln(" aan");
      } else {
        debugln(" uit");
      }
      relays[i]->clearChangedFlag();
    }
  }
#endif
}

void debugknop() {
#if DEBUG == 1
  for (int i = 0; i < 12; i++) {
    if (knoppen[i]->didChange()) {
      debug("knop ");
      debug(i + 1);
      if (knoppen[i]->getValue() == KNOP_INGEDUWD) {
        debugln(": je hebt er op gedrukt!");
      }
      knoppen[i]->clearChangedFlag();
    }
  }
#endif
}


void clearBuffer() {
  while (Serial.available()) {
    Serial.read();
  }
}

void mcp23017Reset(MCP23017& mcp, int IPOL_A, int IPOL_B) {
  debugln(F("reset mcp23017"));
  mcp.init();
  mcp.writeRegister(MCP23017Register::IPOL_A, IPOL_A);
  mcp.writeRegister(MCP23017Register::IPOL_B, IPOL_B);
  mcp.writeRegister(MCP23017Register::GPPU_A, 0);
  mcp.writeRegister(MCP23017Register::GPPU_B, 0);
  mcp.writeRegister(MCP23017Register::GPIO_A, 0);
  mcp.writeRegister(MCP23017Register::GPIO_B, 0);
}

int vindWillekeurigVrijSpoor(int gegevenSpoor, Track tracks[], int arraySize) {
  int matchIndices[arraySize];  // array to store indices of matching elements
  int matchCount = 0;           // count of matching elements

  // Look for matching elements and store their indices in matchIndices
  for (int i = 0; i < arraySize; i++) {
    if (i != gegevenSpoor && tracks[i].state == SpoorStatus::bezet) {
      matchIndices[matchCount++] = i;
    }
  }

  // If we found any matching elements, choose a random index from matchIndices
  if (matchCount > 0) {
    int randomIndex = random(matchCount);
    return matchIndices[randomIndex];
  }

  // If we didn't find any matching elements, return the given track number
  return gegevenSpoor;
}



void setup() {
  Serial.begin(115200);
  debugln("start init");
  debugln("INIT I2C");
  Wire.begin();

  mcp23017Reset(mcp0, 0, 0);
  mcp23017Reset(mcp1, 0, 0);
  mcp23017Reset(mcp2, 0, 0);

  delay(100);
  debugln("init wissels");
  for (Wissel* wissel : wissels) wissel->init();

  debugln(F("INIT led 5-8"));
  for (IO* led : leds) led->init(OUTPUT, 0);

  debugln("INIT knop 1-12");
  for (IO* knop : knoppen) knop->setInput();

  debugln("INIT bezetmelder 1-12");
  for (IO* bezetmelder : bezetmelders) bezetmelder->setInput();

  debugln(F("init leds"));

  for (IO* led : leds) {
    led->setValue(1);
    delay(100);
  }
  for (IO* led : leds) led->setValue(0);
  debugln(F("CLEAR leds"));
  pinMode(5, INPUT);

  debugln(F("init spoorstatus = initialisatie"));
  for (Track track : tracks) track.state = SpoorStatus::initialisatie;

  kopSpoorStatus = (bezetmelder7.getValue() == BEZET) ? KopSpoorStatus::bezet : KopSpoorStatus::vrij;


  debugln(F("EINDE setup"))


    pinMode(peakCurrentResetPin, OUTPUT);
  pinMode(A0, INPUT);
}

#define LED_OFF 0
#define LED_ON 1

#define RELAY_ON 1
#define RELAY_OFF 0

int aantalSporenBezet() {
  int n = 0;
  for (Track track : tracks) {
    if (track.state == SpoorStatus::vrij) n++;
    if (track.state == SpoorStatus::vertrek) return 6;
  }
  return 6 - n;
}

void debugSpoornr(int i) {
  debug(F("spoor "));
  debug(i + 1);
}

void setOutputs(int led, int relay, Richting wissel, int trackNr) {
  leds[trackNr]->setValue(led);
  relays[trackNr]->setValue(relay);

#if KOPSPOOR == 1
  wissels[trackNr]->activate(wissel);
#else
  if (wissels[trackNr] < 4)
    wissels[trackNr]->activate(wissel);
#endif
}

void aantalSporenBezetDebug() {
  switch (aantalSporenBezet()) {
    case 0:
      debugln("alle sporen vrij ");
      break;
    case 1:
      debugln("Een spoor bezet");
      break;
    default:
      debug("er zijn ");
      debug(aantalSporenBezet());
      debugln("sporen bezet");
      break;
  }
}

int lastInrijSpoor = 0;


void loop() {

  debugRelays();
  debugBezetmelders();
  debugknop();

#if KOPSPOOR == 1
  //kopspoor uit: vertrekken
  if (knop10.getValue() == KNOP_INGEDUWD) {
    relay7.setValue(1);
    kopSpoorStatus = KopSpoorStatus::uit;

    debugln("kopspoor uit start");
  }

  //cancel kopspoor uit
  if (knop11.getValue() == KNOP_INGEDUWD || kopSpoorStatus == KopSpoorStatus::uit && bezetmelder7.getValue() == VRIJ) {
    relay7.setValue(0);
    kopSpoorStatus = KopSpoorStatus::vrij;
    debugln("kopspoor uit end");
  }
  //LED kopspoor bezet = continu, vertrek = knipper
  if (tracks[0].state != SpoorStatus::wisselsRechtdoor) {
    led7.setValue((knipper.getValue() && kopSpoorStatus == KopSpoorStatus::uit) || (bezetmelder7.getValue() == BEZET && kopSpoorStatus == KopSpoorStatus::bezet));
  }
#endif

  //aansturen led na uitrijden sporen 1-6 (vrijgave sporen)
  led9.setValue(!bezetmelder9.getValue() == BEZET);

  //veiligheidspoor
  if (aantalSporenBezet() < 6 || (KOPSPOOR == 1 && tracks[0].state == SpoorStatus::wisselsRechtdoor)) {
    relay8.setValue(RELAY_ON);
  } else {
    relay8.setValue(RELAY_OFF);
  }
  //inrijspoor led

  led8.setValue(
    relay8.getValue() == RELAY_ON && bezetmelder8.getValue() == BEZET
    || (relay8.getValue() == RELAY_OFF && knipper.getValue()) && bezetmelder8.getValue() == BEZET);


  //debug start stop inrijspoor
#if DEBUG == 1
  if (relay8.getValue() != lastInrijSpoor) {
    debugln(relay8.getValue() == RELAY_ON ? "inrijspoor start" : "inrijspoor stop");
    lastInrijSpoor = relay8.getValue();
  }
#endif


  for (int i = 0; i < 6; i++) {
    switch (tracks[i].state) {
      case SpoorStatus::vrij:
        setOutputs(LED_OFF, RELAY_OFF, Richting::afbuigend, i);
        if (bezetmelders[i]->getValue() == BEZET) {
          tracks[i].state = SpoorStatus::bezet;
          debugSpoornr(i);
          debugln(F(": bezet"));
          aantalSporenBezetDebug();

          if (knop12.getValue() == KNOP_INGEDUWD) {
            int automatischVertrekSpoor = vindWillekeurigVrijSpoor(i, tracks, 6);
            tracks[automatischVertrekSpoor].state = SpoorStatus::vertrek;
          }
        }
#if KOPSPOOR == 1
        if (knop7.getValue() == KNOP_INGEDUWD && bezetmelder7.getValue() == VRIJ || kopSpoorStatus == KopSpoorStatus::in) {
          tracks[i].state = SpoorStatus::wisselsRechtdoor;
          debugln("status vrij -> wissel rechtdoor");
          kopSpoorStatus = KopSpoorStatus::in;
        }
#endif
        break;

#if KOPSPOOR == 1
      case SpoorStatus::wisselsRechtdoor:
        leds[i]->setHigh();
        delay(150);
        setOutputs(0, RELAY_OFF, Richting::rechtdoor, i);

        if (knop9.getValue() == KNOP_INGEDUWD || bezetmelder7.getValue() == BEZET) {
          tracks[i].state = SpoorStatus::initialisatie;

          int ledValue = (bezetmelder7.getValue() == BEZET) ? 1 : 0;
          led7.setValue(ledValue);

          debugln(bezetmelder7.getValue() == BEZET ? "END kopspoor in" : "cancel kopspoor in");

          kopSpoorStatus = (bezetmelder7.getValue() == BEZET) ? KopSpoorStatus::bezet : KopSpoorStatus::vrij;

          aantalSporenBezetDebug();
        }

        if (i == 5) {
          led7.setValue(!led7.getValue());
        }
        break;
#endif

      case SpoorStatus::bezet:
        setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, i);
        if (knoppen[i]->getValue() == KNOP_INGEDUWD && magVertrekken()) {
          tracks[i].state = SpoorStatus::vertrek;
          debugSpoornr(i);
          debugln(F(": vertrekken"));
          tracks[i].lastDepartureTimestamp = millis();
        }

#if KOPSPOOR == 1
        if ((knop7.getValue() == KNOP_INGEDUWD || kopSpoorStatus == KopSpoorStatus::in) && bezetmelder7.getValue() == VRIJ) {
          tracks[i].state = SpoorStatus::wisselsRechtdoor;
          debugln("status bezet -> wissel rechtdoor");
        }
#endif

        break;
      case SpoorStatus::vertrek:

        setOutputs(knipper.getValue(), RELAY_ON, Richting::rechtdoor, i);
        if (!bezetmelder9.getValue() == BEZET) {
          tracks[i].state = SpoorStatus::vrij;
          aantalSporenBezetDebug();
        }
        break;
      case SpoorStatus::initialisatie:
      default:
        debugSpoornr(i);
        debugln(F(" initialisatie: "));

        if (bezetmelders[i]->getValue() == BEZET) {
          tracks[i].state = SpoorStatus::bezet;
          wissels[i]->zetrechtdoor();
        } else {
          tracks[i].state = SpoorStatus::vrij;
          wissels[i]->zetafbuigend();
        }
        break;
    }
  }
}
