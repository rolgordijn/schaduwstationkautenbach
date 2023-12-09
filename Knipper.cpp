#include "knipper.h"
#include "Arduino.h"

Knipper::Knipper() {
}

Knipper::Knipper(int treshold, int period) {
  this->treshold = treshold;
  this->period = period;
}

bool Knipper::getValue(void) {
  return (millis() % period) > treshold;
}
