#include "IO.h"
#include "Wissel.h"
#include "debug.h"
#include "knipper.h"
#include "debug.h"
#include "IODebugMessage.h"
#include "constants.h"


#define ARRAYCOUNT(x) (sizeof(x) / sizeof(x[0]))



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

MCP23017 mcp2(0x21), mcp1(0x20), mcp0(0x27);

Wissel wissel1 = Wissel(new MCP23017IO(mcp0, 1), new MCP23017IO(mcp0, 0), 1);
Wissel wissel2 = Wissel(new MCP23017IO(mcp0, 3), new MCP23017IO(mcp0, 2), 2);
Wissel wissel3 = Wissel(new MCP23017IO(mcp0, 5), new MCP23017IO(mcp0, 4), 3);
Wissel wissel4 = Wissel(new MCP23017IO(mcp0, 7), new MCP23017IO(mcp0, 6), 4);
Wissel wissel5 = Wissel(new MCP23017IO(mcp0, 9), new MCP23017IO(mcp0, 8), 5);

#if KOPSPOOR == 1
Wissel wissel6 = Wissel(new MCP23017IO(mcp0, 11), new MCP23017IO(mcp0, 10), 6);
#endif

BasicIO led1 = BasicIO(9, OUTPUT);
BasicIO led2 = BasicIO(8, OUTPUT);
BasicIO led3 = BasicIO(7, OUTPUT);
BasicIO led4 = BasicIO(6, OUTPUT);

MCP23017IO led5 = MCP23017IO(mcp0, 12);
MCP23017IO led6 = MCP23017IO(mcp0, 13);
MCP23017IO led7 = MCP23017IO(mcp0, 14);
MCP23017IO led8 = MCP23017IO(mcp0, 15);

MCP23017IO led9 = MCP23017IO(mcp2, 8);
MCP23017IO led10 = MCP23017IO(mcp2, 9);
MCP23017IO led11 = MCP23017IO(mcp2, 10);
MCP23017IO led12 = MCP23017IO(mcp2, 11);
MCP23017IO led13 = MCP23017IO(mcp2, 12);
MCP23017IO led14 = MCP23017IO(mcp2, 13);
MCP23017IO led15 = MCP23017IO(mcp2, 14);
MCP23017IO led16 = MCP23017IO(mcp2, 15);

MCP23017IO knop1 = MCP23017IO(mcp1, 0);
MCP23017IO knop2 = MCP23017IO(mcp1, 1);
MCP23017IO knop3 = MCP23017IO(mcp1, 2);
MCP23017IO knop4 = MCP23017IO(mcp1, 3);
MCP23017IO knop5 = MCP23017IO(mcp1, 4);
MCP23017IO knop6 = MCP23017IO(mcp1, 5);
MCP23017IO knop7 = MCP23017IO(mcp1, 6);
MCP23017IO knop8 = MCP23017IO(mcp1, 7);

MCP23017IO bezetmelder9 = MCP23017IO(mcp2, 0);
MCP23017IO bezetmelder10 = MCP23017IO(mcp2, 1);
MCP23017IO bezetmelder11 = MCP23017IO(mcp2, 2);
MCP23017IO bezetmelder12 = MCP23017IO(mcp2, 3);
MCP23017IO knop9 = MCP23017IO(mcp2, 4);
MCP23017IO knop10 = MCP23017IO(mcp2, 5);
MCP23017IO knop11 = MCP23017IO(mcp2, 6);
MCP23017IO knop12 = MCP23017IO(mcp2, 7);

MCP23017IO bezetmelder1 = MCP23017IO(mcp1, 8);
MCP23017IO bezetmelder2 = MCP23017IO(mcp1, 9);
MCP23017IO bezetmelder3 = MCP23017IO(mcp1, 10);
MCP23017IO bezetmelder4 = MCP23017IO(mcp1, 11);
MCP23017IO bezetmelder5 = MCP23017IO(mcp1, 12);
MCP23017IO bezetmelder6 = MCP23017IO(mcp1, 13);
MCP23017IO bezetmelder7 = MCP23017IO(mcp1, 14);
MCP23017IO bezetmelder8 = MCP23017IO(mcp1, 15);

BasicIO relay1 = BasicIO(10, OUTPUT);
BasicIO relay2 = BasicIO(11, OUTPUT);
BasicIO relay3 = BasicIO(12, OUTPUT);
BasicIO relay4 = BasicIO(13, OUTPUT);
BasicIO relay5 = BasicIO(A3, OUTPUT);
BasicIO relay6 = BasicIO(A2, OUTPUT);
BasicIO relay7 = BasicIO(A1, OUTPUT);
BasicIO relay8 = BasicIO(2, OUTPUT);

BasicIO auxSwitch = BasicIO(5, INPUT);

//StopWatch stopWatch = StopWatch();
Knipper knipper = Knipper(100, 500);


#if KOPSPOOR == 1
Wissel* wissels[] = { &wissel1, &wissel2, &wissel3, &wissel4, &wissel5, &wissel6 };
#else
Wissel* wissels[] = { &wissel1, &wissel2, &wissel3, &wissel4, &wissel5 };
#endif

// input
IO* knoppen[] = { &knop1, &knop2, &knop3, &knop4, &knop5, &knop6, &knop7, &knop8, &knop9, &knop10, &knop11, &knop12 };
IO* bezetmelders[] = { &bezetmelder1, &bezetmelder2, &bezetmelder3, &bezetmelder4, &bezetmelder5, &bezetmelder6, &bezetmelder7, &bezetmelder8, &bezetmelder9, &bezetmelder10, &bezetmelder11, &bezetmelder12 };

//Ouput
IO* relays[] = { &relay1, &relay2, &relay3, &relay4, &relay5, &relay6, &relay7, &relay8 };
IO* leds[] = { &led1, &led2, &led3, &led4, &led5, &led6, &led7, &led8, &led9, &led10, &led11, &led12, &led13, &led14, &led15, &led16 };

Track uitrijspoor;

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


int aantalSporenMetStatus(SpoorStatus status) {
  int n = 0;
  for (Track track : tracks) {
    if (track.state == status) n++;
  }
  return n;
}

void debugIOPins(IO** ioArray, size_t ioCount, bool executeGetValue) {
  for (int i = 0; i < ioCount; i++) {
    if (executeGetValue) ioArray[i]->getValue();
    if (ioArray[i]->didChange()) {
      ioArray[i]->printDebugMsg(Serial);
      ioArray[i]->clearChangedFlag();
    }
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

int vindWillekeurigBezetSpoor(int gegevenSpoor, Track tracks[], int arraySize) {
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

void initializeI2C() {
  debugln("INIT I2C");
  Wire.begin();
}


void initializeTracks() {
  debugln(F("init spoorstatus = initialisatie"));
  for (Track& track : tracks) {
    track.state = SpoorStatus::initialisatie;
  }
}

void initializeWissels() {
  debugln("init wissels");
  for (Wissel* wissel : wissels) {
    wissel->init();
  }
}


void initializeIOArray(IO* ios[], size_t count, const char* basename, const char* onMessage, const char* offMessage, int tris, int level = 0) {
  char debugMessage[50];  // Adjust size as needed
  snprintf(debugMessage, sizeof(debugMessage), "INIT %s", basename);
  debugln(debugMessage);
  for (int i = 0; i < count; i++) {
    char name[20];  // Array size 10 is sufficient for "led " + up to 2 digits (e.g., "led 16")
    snprintf(name, sizeof(name), "%s %d", basename, i + 1);
    ios[i]->setDebugMessage(new IODebugMessage(name, onMessage, offMessage));
    ios[i]->init(tris, level);
  }
}


void checkLeds() {
  debugln(F("init leds"));
  for (IO* led : leds) {
    led->setValue(1);
    delay(100);
  }
  for (IO* led : leds) {
    led->setValue(0);
  }
  debugln(F("CLEAR leds"));
}


void initializePins() {
  pinMode(peakCurrentResetPin, OUTPUT);
  pinMode(A0, INPUT);
}

void initializeKopSpoorStatus() {

#if KOPSPOOR == 1
  kopSpoorStatus = (bezetmelder7.getValue() == BEZET) ? KopSpoorStatus::bezet : KopSpoorStatus::vrij;
#endif
}

void initializeUitrijspoor() {
  uitrijspoor.state = (bezetmelder9.getValue() != BEZET) ? SpoorStatus::bezet : SpoorStatus::vrij;
  uitrijspoor.lastDepartureTimestamp = millis();
  debug("Uitrijspoor timestamp ms :");
  debugln(uitrijspoor.lastDepartureTimestamp);
}

void initUART() {
  Serial.begin(115200);
  debugln("start init");
}


void setup() {
  initUART();
  initializeI2C();
  mcp23017Reset(mcp0, 0, 0);
  mcp23017Reset(mcp1, 0, 0xFF);
  mcp23017Reset(mcp2, 0, 0XFF);
  delay(100);
  initializeWissels();
  checkLeds();
  initializeTracks();
  initializeKopSpoorStatus();
  initializePins();
  initializeUitrijspoor();
  auxSwitch.setInput();

  initializeIOArray(leds, ARRAYCOUNT(leds), "Led", "is aan", "is uit", OUTPUT);
  initializeIOArray(knoppen, ARRAYCOUNT(knoppen), "Knop", "Je hebt erop gedrukt", "Je hebt de knop losgelaten", INPUT);
  initializeIOArray(relays, ARRAYCOUNT(relays), "Relais", "is aan", "is uit", OUTPUT);
  initializeIOArray(bezetmelders, ARRAYCOUNT(bezetmelders), "bezetmelder", "is bezet", "is vrij", INPUT);

  relay8.setDebugMessage(new IODebugMessage("Inrijspoor relais 8", ": start", ": stop"));

  debugln(F("==========EINDE setup=========="));
}


int indexVanEersteSpoorMetStatus(SpoorStatus status) {
  for (int i = 0; i < 6; i++) {
    if (tracks[i].state == status) {
      return i;
    }
  }
  return -1;
}

int aantalSporenBezet() {
  int n = 0;
  for (Track track : tracks) {
    if (track.state == SpoorStatus::vrij) n++;
    if (track.state == SpoorStatus::vertrek) return 6;
  }
  return 6 - n;
}

void debugSpoor(int i, const char* status) {
  char buffer[100];  // Adjust size as needed to accommodate both parts of the message

  // Format the base message with the dynamic part
  snprintf(buffer, sizeof(buffer), "spoor %d%s", i + 1, status);

  // Print the complete message
  debugln(buffer);
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
      debugln(" sporen bezet");
      break;
  }
}

void inrijspoorLED(IO& relay, IO& bezetmelder, bool knipperValue, IO& led) {
  if (relay.getValue() == RELAY_ON && bezetmelder.getValue() == BEZET) {
    led.setValue(LED_ON);
  } else if (relay.getValue() == RELAY_OFF && knipperValue && bezetmelder.getValue() == BEZET) {
    led.setValue(knipperValue);
  } else {
    led.setValue(knipperValue);
  }
}

void inrijspoorRelais() {
  int eersteVrijSpoor = indexVanEersteSpoorMetStatus(SpoorStatus::vrij);
  int vertrekSpoornummer = indexVanEersteSpoorMetStatus(SpoorStatus::vertrek);
  bool naarKopSpoor = indexVanEersteSpoorMetStatus(SpoorStatus::wisselsRechtdoor) == 0 ? true : false;

  if (vertrekSpoornummer != -1 && vertrekSpoornummer < eersteVrijSpoor) {
    relay8.setLow();
  } else if (eersteVrijSpoor == -1 && naarKopSpoor == false) {
    relay8.setLow();
  } else {
    relay8.setHigh();
  }
}


bool automatischVertrekken = false;

bool magVertrekken() {
  if (aantalSporenMetStatus(SpoorStatus::vertrek)) return false;  //niet vertrekken als een andere trein al vertrekt
  if (kopSpoorStatus == KopSpoorStatus::uit) return false;        //niet vertrekken als kopspoortrein vertrekt
  if (uitrijspoor.state == SpoorStatus::bezet) return false;      //
  if (uitrijspoor.state == SpoorStatus::vertrek) return false;
  return true;
}


void uitrijspoorLed() {
  //aansturen led na uitrijden sporen 1-6 (vrijgave sporen)
  switch (uitrijspoor.state) {
    case SpoorStatus::vertrek:
      led9.setValue(knipper.getValue());
      break;
    case SpoorStatus::bezet:
      led9.setValue(LED_ON);
      break;
    case SpoorStatus::vrij:
      led9.setValue(LED_OFF);
      break;
    default:
      break;
  }
}


void uitrijspoorLogica() {
  if ((uitrijspoor.state == SpoorStatus::bezet) && (bezetmelder9.getValue() == BEZET)) {
    uitrijspoor.lastDepartureTimestamp = millis();
    uitrijspoor.state = SpoorStatus::vertrek;
    debugln("Uitrijspoor status: vertrek");
    debug("Uitrijspoor timestamp ms :");
    debugln(uitrijspoor.lastDepartureTimestamp);
  }

  if ((uitrijspoor.state == SpoorStatus::vertrek) && (bezetmelder9.getValue() == BEZET)) {
    uitrijspoor.lastDepartureTimestamp = millis();
    uitrijspoor.state = SpoorStatus::vertrek;
  }

  if ((uitrijspoor.state == SpoorStatus::vertrek) && (bezetmelder9.getValue() == VRIJ) && (millis() - uitrijspoor.lastDepartureTimestamp > 15000)) {
    uitrijspoor.state = SpoorStatus::vrij;
    debugln("Uitrijspoor status: vrij");
  }
}

void loop() {
  if (auxSwitch.getValue()) {

    debugIOPins(knoppen, ARRAYCOUNT(knoppen), false);
    debugIOPins(relays, ARRAYCOUNT(relays), false);
    debugIOPins(bezetmelders, ARRAYCOUNT(bezetmelders), false);

    uitrijspoorLed();
    uitrijspoorLogica();


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

    inrijspoorLED(relay8, bezetmelder8, knipper.getValue(), led8);
    inrijspoorRelais();

    for (int i = 0; i < 6; i++) {
      switch (tracks[i].state) {
        case SpoorStatus::vrij:
          setOutputs(LED_OFF, RELAY_OFF, Richting::afbuigend, i);
          if (bezetmelders[i]->getValue() == BEZET) {

            tracks[i].state = SpoorStatus::bezet;
            debugSpoor(i, "bezet");
            aantalSporenBezetDebug();

            if (knop12.getValue() == KNOP_INGEDUWD && magVertrekken()) {
              automatischVertrekken = true;
              int automatischVertrekSpoor = vindWillekeurigBezetSpoor(i, tracks, 6);
              tracks[automatischVertrekSpoor].state = SpoorStatus::vertrek;
              uitrijspoor.state = SpoorStatus::bezet;
            }
          }
          if (knop7.getValue() == KNOP_INGEDUWD && bezetmelder7.getValue() == VRIJ || kopSpoorStatus == KopSpoorStatus::in) {
            tracks[i].state = SpoorStatus::wisselsRechtdoor;
            debugln("status vrij -> wissel rechtdoor");
            kopSpoorStatus = KopSpoorStatus::in;
          }
          break;
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

        case SpoorStatus::bezet:
          setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, i);
          if (knoppen[i]->getValue() == KNOP_INGEDUWD && magVertrekken()) {
            tracks[i].state = SpoorStatus::vertrek;
            debugSpoor(i, "vertrekken");
            tracks[i].lastDepartureTimestamp = millis();
            uitrijspoor.state = SpoorStatus::bezet;
            debugln(F("Uitrijspoor :bezet"))
          }


          if ((knop7.getValue() == KNOP_INGEDUWD || kopSpoorStatus == KopSpoorStatus::in) && bezetmelder7.getValue() == VRIJ) {
            tracks[i].state = SpoorStatus::wisselsRechtdoor;
            debugln("status bezet -> wissel rechtdoor");
          }

          break;
        case SpoorStatus::vertrek:
          if (knop12.getValue() == KNOP_NIET_INGEDUWD && automatischVertrekken == true) {
            tracks[i].state = SpoorStatus::bezet;
            automatischVertrekken = false;
          }
          setOutputs(knipper.getValue(), RELAY_ON, Richting::rechtdoor, i);
          if (uitrijspoor.state == SpoorStatus::vrij) {
            tracks[i].state = SpoorStatus::vrij;
            aantalSporenBezetDebug();
            automatischVertrekken = false;
          }

          break;
        case SpoorStatus::initialisatie:
        default:
          debugSpoor(i, "initialisatie");

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
  } else {




    for (int i = 0; i < 6; i++) {
      delay(50);
      if (i == 0 || i == 3) {
        led9.setValue(!led9.getValue());
        led8.setValue(!led9.getValue());
      }
      if (i == 2 || i == 5) {
        led7.setValue(!led9.getValue());
      }

      while (knoppen[i]->getValue() == KNOP_INGEDUWD) {
        wissels[i]->activate(Richting::rechtdoor);
        leds[i]->setHigh();
        delay(500);
        wissels[i]->activate(Richting::afbuigend);
        leds[i]->setLow();
        delay(500);
      }
    }
  }
}
