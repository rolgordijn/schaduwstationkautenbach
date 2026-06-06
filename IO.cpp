#include "IO.h"

IO::IO() {
  pin = 0;
  pinState = 0;
  changed = false;
}

void IO::setHigh() {
  setValue(1);
}

void IO:: setLow() {
  setValue(0);
}

void IO::clearChangedFlag() {
  this->changed = false;
}

bool IO::didChange(void) {
  return this->changed;
}
