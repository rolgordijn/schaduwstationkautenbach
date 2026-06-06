#include "Kopspoor.h"
#include "Arduino.h"
#include "debug.h"

Kopspoor::Kopspoor(IO& sensor, IO& relais, IO& led,
                   IO& btnIn, IO& btnAnnuleer,
                   IO& btnUit, IO& btnAnnuleerUit,
                   Knipper& knipper)
    : sensor(sensor), relais(relais), led(led),
      btnIn(btnIn), btnAnnuleer(btnAnnuleer),
      btnUit(btnUit), btnAnnuleerUit(btnAnnuleerUit),
      knipper(knipper),
      status(KopspoorStatus::vrij),
      animStep(0), lastAnimTick(0), ledAan(false) {}

void Kopspoor::init() {
    relais.init(OUTPUT, LOW);
    led.init(OUTPUT, LOW);
    sensor.setInput();
    btnIn.setInput();
    btnAnnuleer.setInput();
    btnUit.setInput();
    btnAnnuleerUit.setInput();
}

void Kopspoor::transitionTo(KopspoorStatus s) {
    status = s;
}

void Kopspoor::update(bool (*magVertrekken)()) {
    switch (status) {

        case KopspoorStatus::vrij:
            led.setValue(LED_OFF);
            if (btnIn.getValue() == KNOP_INGEDUWD && sensor.getValue() == VRIJ) {
                // Operator signals a train is about to enter; animation lets Track LEDs sweep to confirm switches are set
                animStep    = 0;
                lastAnimTick = millis();
                ledAan      = false;
                transitionTo(KopspoorStatus::inRijden);
                debugln(F("kopspoor in: start"));
            } else if (sensor.getValue() == BEZET) {
                // Train entered without button (shunting move) — skip animation, go straight to bezet
                transitionTo(KopspoorStatus::bezet);
            }
            break;

        case KopspoorStatus::inRijden:
            if (btnAnnuleer.getValue() == KNOP_INGEDUWD) {
                led.setValue(LED_OFF);
                transitionTo(KopspoorStatus::vrij);
                debugln(F("kopspoor in: geannuleerd"));
            } else if (sensor.getValue() == BEZET) {
                led.setValue(LED_ON);
                transitionTo(KopspoorStatus::bezet);
                debugln(F("kopspoor in: trein aangekomen"));
            } else {
                // animStep drives the track LED sweep via getAnimStep(); ledAan alternates the kopspoor LED each full cycle
                if (millis() - lastAnimTick >= ANIM_STEP_MS) {
                    lastAnimTick = millis();
                    if (++animStep >= ANIM_STEPS) {
                        animStep = 0;
                        ledAan   = !ledAan;
                    }
                }
                led.setValue(ledAan ? LED_ON : LED_OFF);
            }
            break;

        case KopspoorStatus::bezet:
            led.setValue(LED_ON);
            // magVertrekken() blocks departure while a regular track is also departing — ladder can only hold one train
            if (btnUit.getValue() == KNOP_INGEDUWD && magVertrekken()) {
                relais.setValue(RELAY_ON);
                transitionTo(KopspoorStatus::uitRijden);
                debugln(F("kopspoor uit: start"));
            }
            break;

        case KopspoorStatus::uitRijden:
            led.setValue(knipper.getValue() ? LED_ON : LED_OFF);
            if (sensor.getValue() == VRIJ) {
                // Tail of train has cleared — safe to cut relay power now
                relais.setValue(RELAY_OFF);
                transitionTo(KopspoorStatus::vrij);
                debugln(F("kopspoor uit: einde"));
            } else if (btnAnnuleerUit.getValue() == KNOP_INGEDUWD) {
                relais.setValue(RELAY_OFF);
                transitionTo(sensor.getValue() == BEZET
                             ? KopspoorStatus::bezet
                             : KopspoorStatus::vrij);
                debugln(F("kopspoor uit: geannuleerd"));
            }
            break;
    }
}

KopspoorStatus Kopspoor::getStatus()   const { return status; }
bool           Kopspoor::isInRijden()  const { return status == KopspoorStatus::inRijden; }
bool           Kopspoor::isUitRijden() const { return status == KopspoorStatus::uitRijden; }
int            Kopspoor::getAnimStep() const { return animStep; }
