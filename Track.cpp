#include "Track.h"
#include "IO.h"
#include "Wissel.h"

// Constructor definition
Track::Track(IO& relay, IO& knop, IO& bezetmelder, Wissel& wissel, IO& led, TrackState trackStatus)
  : relay(relay), knop(knop), bezetmelder(bezetmelder), wissel(wissel), led(led), trackStatus(trackStatus) {
  // Additional initialization if needed
}

/**
void Track::update(void) {

  switch (this->trackStatus) {
    case SpoorStatus::vrij:
      setOutputs(LED_OFF, RELAY_OFF, Richting::afbuigend, i);
      if (bezetmelders[i]->getValue() == BEZET) {
        this->trackStatus = SpoorStatus::bezet;
        debugSpoornr(i);
        debugln(F(": bezet"));
        aantalSporenBezetDebug();

        if (knop12.getValue() == KNOP_INGEDUWD) {
          int automatischVertrekSpoor = vindWillekeurigVrijSpoor(i, spoorStatus, 6);
          if (automatischVertrekSpoor == i) delay(2500);
          spoorStatus[automatischVertrekSpoor] = SpoorStatus::vertrek;
        }
      }
#if KOPSPOOR == 1
      if (knop7.getValue() == KNOP_INGEDUWD && bezetmelder7.getValue() == VRIJ || kopspoorIn == true) {
        this->trackStatus = SpoorStatus::wisselsRechtdoor;
        debugln("status vrij -> wissel rechtdoor");
        kopspoorIn = true;
      }
#endif
      break;

#if KOPSPOOR == 1
    case SpoorStatus::wisselsRechtdoor:
      leds[i]->setHigh();
      delay(150);
      setOutputs(0, RELAY_OFF, Richting::rechtdoor, i);

      if (knop9.getValue() == KNOP_INGEDUWD || bezetmelder7.getValue() == BEZET) {
        this->trackStatus) = SpoorStatus::initialisatie;
        led7.setValue(bezetmelder7.getValue() == BEZET ? 1 : 0);
        debugln(bezetmelder7.getValue() == BEZET ? "END kopspoor in" : "cancel kopspoor in");
        kopspoorIn = false;
        aantalSporenBezetDebug();
      }

      if (i == 5) {
        led7.setValue(!led7.getValue());
      }
      break;
#endif

    case SpoorStatus::bezet:
      setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, i);
      if (knoppen[i]->getValue() == KNOP_INGEDUWD) {
        this->trackStatus) = SpoorStatus::vertrek;
        debugSpoornr(i);
        debugln(F(": vertrekken"));
      }

#if KOPSPOOR == 1
      if ((knop7.getValue() == KNOP_INGEDUWD || kopspoorIn == true) && bezetmelder7.getValue() == VRIJ) {
        this->trackStatus) = SpoorStatus::wisselsRechtdoor;
        debugln("status bezet -> wissel rechtdoor");
      }
#endif

      break;
    case SpoorStatus::vertrek:

      setOutputs(knipper.getValue(), RELAY_ON, Richting::rechtdoor, i);
      if (!bezetmelder9.getValue() == BEZET) {
        this->trackStatus) = SpoorStatus::vrij;
        aantalSporenBezetDebug();
      }
      break;
    case SpoorStatus::initialisatie:
    default:
      debugSpoornr(i);
      debugln(F(" initialisatie: "));

      if (bezetmelders[i]->getValue() == BEZET) {
        this->trackStatus) = SpoorStatus::bezet;
        wissels[i]->zetrechtdoor();
      } else {
        this->trackStatus) = SpoorStatus::vrij;
        wissels[i]->zetafbuigend();
      }
      break;
  }
}
}
}*/