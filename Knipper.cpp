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


/*
#include "knipper.h"
#include "Arduino.h"

Knipper::Knipper() {
  // Initialize the timer and set the interrupt to trigger every 1000 milliseconds
  Timer1.initialize(1000);
  Timer1.attachInterrupt(toggleFlag);
}

Knipper::Knipper(int treshold, int period) {
  // Initialize the timer and set the interrupt to trigger every "period" milliseconds
  Timer1.initialize(period);
  Timer1.attachInterrupt(toggleFlag);
  this->treshold = treshold;
  this->period = period;
}

void Knipper::toggleFlag() {
  flag = !flag;
}

bool Knipper::getValue(void) {
  return flag;
}#$%*/ 
