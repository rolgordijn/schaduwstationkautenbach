#include "Knipper.h"
#include "Arduino.h"

Knipper::Knipper() : treshold(500), period(1000) {
}

Knipper::Knipper(int treshold, int period) {
  this->treshold = treshold;
  this->period = period;
}

bool Knipper::getValue(void) {
  return (millis() % period) > treshold;
}
