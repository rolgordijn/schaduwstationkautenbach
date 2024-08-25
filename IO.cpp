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
  debugMsg->updateMessage(1);
}

void IO:: setLow() {
  setValue(0);
  debugMsg->updateMessage(0);
}

void IO::clearChangedFlag() {
  this->changed = false;
}

bool IO::didChange(void) {
  return this->changed;
}

void IO::setDebugMessage(IODebugMessage* debugMsg){
    this->debugMsg = debugMsg;
 }

void IO::printDebugMsg(Print &printer = Serial){
  printer.println(debugMsg->getMessage());
}
