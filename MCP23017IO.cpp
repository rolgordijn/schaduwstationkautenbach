#include "io.h"

MCP23017IO::MCP23017IO(): IO() {
  //Serial.println(F("default constructor MCP23017IO"));
  this->pin = 0;
}

MCP23017IO::MCP23017IO(MCP23017 * ic, uint8_t pin): IO() {
  //Serial.println(F("constructor MCP23017IO 1"));
  this->pin = pin;
  this->ic = ic;
}

void MCP23017IO::init(int dir, int level) {
  this->setValue(level);
  this->setPinMode(dir);
}

void MCP23017IO::setHigh(void) {
  if (pinState == HIGH) return;
  ic->digitalWrite(pin, HIGH);
  this->pinState = HIGH;
  this->changed = true;
}

void MCP23017IO::setLow(void) {
  if (pinState == LOW) return;
  ic->digitalWrite(pin, LOW);
  this->pinState = LOW;
  this->changed = true;
}

bool MCP23017IO::getValue(void) {
  bool pinState = ic -> digitalRead(pin);
  if (pinState != this->pinState) {
    this->changed = true;
  }
  this->pinState = pinState;
  return pinState;
}

void MCP23017IO::setValue(bool val) {
  ic->digitalWrite(pin, val);
}

void MCP23017IO::setInput() {
  ic->pinMode(pin, INPUT);
}
void MCP23017IO::setOutput() {
  ic->pinMode(pin, OUTPUT);
}

void MCP23017IO::setPinMode(int dir) {
  if (dir == INPUT) {
    this->setInput();
  }
  else {
    this->setOutput();
  }
}

int MCP23017IO::getPinMode(void) {
  return 0; // not implemented yet? How?
}
