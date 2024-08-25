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

[[deprecated("Use turnOn() instead")]] void IO::setHigh(void) {
  if ((pinState ^ invertedLogic) == HIGH) return;
  setPinState(HIGH ^ invertedLogic);
  pinState = HIGH ^ invertedLogic;
  changed = true;
}

void IO::turnOn(void) {
  if ((pinState ^ invertedLogic) == HIGH) return;
  setPinState(HIGH ^ invertedLogic);
  pinState = HIGH ^ invertedLogic;
  changed = true;
}


[[deprecated("Use turnOff() instead")]] void IO::setLow(void) {
  if ((pinState ^ invertedLogic) == LOW) return;
  setPinState(pinState ^ invertedLogic);
  pinState = LOW ^ invertedLogic;
  changed = true;
}

void IO::turnOff(void) {
  if ((pinState ^ invertedLogic) == LOW) return;
  setPinState(pinState ^ invertedLogic);
  pinState = LOW ^ invertedLogic;
  changed = true;
}

void IO::clearChangedFlag() {
  changed = false;
}

bool IO::didChange(void) {
  return changed;
}

void IO::setInvertedLogic(bool invert) {
  invertedLogic = invert;
}

bool IO::isInvertedLogic() {
  return invertedLogic;
}

int IO::getValue(void) {
  bool currentPinState = readPinState() ^ invertedLogic;
  if (currentPinState != pinState) {
    this->changed = true;
    this->pinState = currentPinState;
  }
  return pinState;
}

void IO::setNameAndIndex(String pinName, int pinIndex) {
  name = pinName;
  index = pinIndex;
}

void IO::debug() {
  //const char* state = (pinState == (invertedLogic ? LED_ON : LED_OFF)) ? "ON" : "OFF";
  Serial.print(name);
  Serial.print(index);
  Serial.print(f("is ");
  Serial.println("lol");
}
