#include "IO.h"

IO::IO() {
  changed = true;
  pin = 0 ;
  tris = 1;
  pinState = 0;
  changed = 0;
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
