#include "TrackState.h"
#include "Track.h"


void VrijState::handle(Track& track) override {
  // Handle the 'vrij' state behavior
  setOutputs(LED_OFF, RELAY_OFF, Richting::afbuigend, track.index);
  if (bezetmelders[track.index]->getValue() == BEZET) {
    track.updateStatus(SpoorStatus::bezet);
    track.setState(new BezetState());
    debugSpoor(track.index, "bezet");
    aantalSporenBezetDebug();
  }



  void BezetState::handle(Track & track) override {
    // Handle the 'bezet' state behavior
    setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, track.index);
    if (knoppen[track.index]->getValue() == KNOP_INGEDUWD && magVertrekken()) {
      track.updateStatus(SpoorStatus::vertrek);
      track.setState(new VertrekState());
      debugSpoor(track.index, "vertrekken");
      track.lastDepartureTimestamp = millis();
      uitrijspoor.state = SpoorStatus::bezet;
      debugln(F("Uitrijspoor :bezet"));
    }
  }


  void VertrekState::handle(Track & track) override {
    // Handle the 'bezet' state behavior
    setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, track.index);
    if (knoppen[track.index]->getValue() == KNOP_INGEDUWD && magVertrekken()) {
      track.updateStatus(SpoorStatus::vertrek);
      track.setState(new VertrekState());
      debugSpoor(track.index, "vertrekken");
      track.lastDepartureTimestamp = millis();
      uitrijspoor.state = SpoorStatus::bezet;
      debugln(F("Uitrijspoor :bezet"));
    }
  }




  void WisselsRechtdoorState::handle(Track & track) override {
    // Handle the 'bezet' state behavior
    setOutputs(LED_ON, RELAY_OFF, Richting::rechtdoor, track.index);
    if (knoppen[track.index]->getValue() == KNOP_INGEDUWD && magVertrekken()) {
      track.updateStatus(SpoorStatus::vertrek);
      track.setState(new VertrekState());
      debugSpoor(track.index, "vertrekken");
      track.lastDepartureTimestamp = millis();
      uitrijspoor.state = SpoorStatus::bezet;
      debugln(F("Uitrijspoor :bezet"));
    }
  }

  // Implement other state classes (VertrekState, WisselsRechtdoorState, etc.)
