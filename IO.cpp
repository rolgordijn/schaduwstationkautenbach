#include "IO.h"

IO::IO() {
  changed = true;
  pin = 0;
  tris = 1;
  pinState = 0;
  changed = 0;

  debugMsg = nullptr;
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
  if (pinState == HIGH) return;
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
    handleStateChange(currentPinState);
  }
  return pinState;
}

void IO::handleStateChange(bool currentPinState) {
  if (ioEventHandler) {
    triggerEvent(currentPinState);
  } else {
    this->changed = true;  // Specific for polling
  }
  updateState(currentPinState);
  updateDebugMessage(currentPinState);
}

void IO::triggerEvent(bool currentPinState) {
  Event e = currentPinState ? Event::RISING_EDGE : Event::FALLING_EDGE;
  ioEventHandler(index, e);
}

void IO::updateState(bool currentPinState) {
  this->pinState = currentPinState;
}

void IO::updateDebugMessage(bool currentPinState) {
  debugMsg->updateMessage(currentPinState);
}


void IO::setDebugMessage(IODebugMessage* debugMsg) {
  this->debugMsg = debugMsg;
}

void IO::printDebugMsg(Print& printer) {
  if (debugMsg != nullptr) {
    printer.println(debugMsg->getMessage());
  } else {
    printer.println("No debug message configured.");
  }
}

void IO::setIndex(int index) {
  this->index = index;
}

int IO::getIndex(void) {
  return index;
}


void IO::setCallback(void (*ioEventHandler)(int, Event)) {
  this->ioEventHandler = ioEventHandler;
}