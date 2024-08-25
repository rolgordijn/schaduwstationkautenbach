#include "io.h"

MCP23017IO::MCP23017IO()
  : IO() {
  //Serial.println(F("default constructor MCP23017IO"));
  this->pin = 0;
}

MCP23017IO::MCP23017IO(MCP23017* ic, uint8_t pin)
  : IO() {
  //Serial.println(F("constructor MCP23017IO 1"));
  this->pin = pin;
  this->ic = ic;
}


void MCP23017IO::setPinState(bool level){
     ic->digitalWrite(pin, level);
}

bool MCP23017IO::readPinState() const{
    return ic->digitalRead(pin);
}

void MCP23017IO::setInput() {
  ic->pinMode(pin, INPUT);
}
void MCP23017IO::setOutput() {
  ic->pinMode(pin, OUTPUT);
}

int MCP23017IO::getPinMode(void) {
  return 0;  // not implemented yet? How?
}
