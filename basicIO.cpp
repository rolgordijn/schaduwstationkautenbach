#include "IO.h"

BasicIO::BasicIO(): IO() {
  ///Serial.println(F("BasicIO default constructor"));
}

BasicIO::BasicIO(int pin, int tris, int val): IO() {
  //Serial.println(F("BasicIO constructor 1") );
  this->pin = pin;
  pinMode(pin, tris);
  digitalWrite(pin, val);
}

BasicIO::BasicIO(int pin, int tris): IO() {
  //Serial.println(F("BasicIO constructor 1") );
  this->pin = pin;
  pinMode(pin, tris);
  digitalWrite(pin, 0);
}
void BasicIO::setHigh(void) {
  if (pinState == HIGH) return;
  digitalWrite(pin, HIGH);
  this->pinState = HIGH;
  this->changed = true;
}
void BasicIO::setLow(void) {
  if (pinState == LOW) return;
  this->changed = true;
  digitalWrite(pin, LOW);
  this->pinState = LOW;
}

bool BasicIO::getValue() {
  bool pinState = digitalRead(pin);
  if (pinState != this->pinState) {
    this->changed = true;
  }
  this->pinState = pinState;
  return pinState;
}

void BasicIO::setValue(bool val) {
  if (val) {
    this->setHigh();
  } else {
    this->setLow();
  }
}

void BasicIO::setInput() {
  pinMode(pin, INPUT);
}
void BasicIO::setOutput() {
  pinMode(pin, OUTPUT);
}

void BasicIO::setPinMode(int dir) {
  pinMode(pin, dir);
}

int BasicIO::getPinMode(void) {
  return 0; // not implemented yet? How?
}

void BasicIO::init(int dir, int level){
  this->setPinMode(dir);
  this->setValue(level);
  this->getValue();
  this->clearChangedFlag();
}
