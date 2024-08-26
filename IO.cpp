#include "IO.h"

IO::IO() {
  changed = true;
  pin = 0;
  tris = 1;
  pinState = 0;
  changed = 0;
}

void IO::init(int dir, int level) {
  setValue(level);
  setPinMode(dir);
}

void IO::setValue(bool val) {
  val ? this->setHigh() : this->setLow();
}

void IO::setPinMode(int dir) {
  (dir == INPUT) ? this->setInput() : this->setOutput();
}

void IO::setHigh(void) {
  if ((pinState == HIGH) return;
  setPinState(HIGH);
  pinState = HIGH;
  changed = true;
  debugMsg->updateMessage(1);
}
void IO::setLow(void) {
  if (pinState == LOW) return;
  setPinState(LOW);
  pinState = LOW;
  changed = true;
   debugMsg->updateMessage(1);

}

void IO::clearChangedFlag() {
  changed = false;
}

bool IO::didChange(void) {
  return changed;
}

int IO::getValue(void) {
  bool currentPinState = readPinState();
  if (currentPinState != pinState) {
    this->changed = true;
    this->pinState = currentPinState;
    debugMsg->updateMessage(currentPinState);
  }
  return pinState;
}

void IO::setDebugMessage(IODebugMessage* debugMsg){
    this->debugMsg = debugMsg;
 }

void IO::printDebugMsg(Print &printer = Serial){
  printer.println(debugMsg->getMessage());
}
