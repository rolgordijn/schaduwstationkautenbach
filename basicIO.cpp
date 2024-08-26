#include "Arduino.h"
#include "IO.h"

BasicIO::BasicIO()  : IO() {
  ///Serial.println(F("BasicIO default constructor"));
}

BasicIO::BasicIO(int pin, int tris, int val): IO() {
  //Serial.println(F("BasicIO constructor 1") );
  this->pin = pin;
  pinMode(pin, tris);
  digitalWrite(pin, val);
}

BasicIO::BasicIO(int pin, int tris) : IO() {
  //Serial.println(F("BasicIO constructor 1") );
  this->pin = pin;
  pinMode(pin, tris);
  digitalWrite(pin, 0);
}


bool BasicIO::readPinState() const {
  return digitalRead(pin);
}

void BasicIO::setPinState(bool level) {
  digitalWrite(pin, level);
}

void BasicIO::setInput() {
  pinMode(pin, INPUT);
}
void BasicIO::setOutput() {
  pinMode(pin, OUTPUT);
}

int BasicIO::getPinMode(void) {
  return 0;  // not implemented yet? How?
}
