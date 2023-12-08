#include "Wissel.h"
#include "IO.h"
#include "debug.h"

Wissel::Wissel() {
  //Serial.println(F("default constructor wissel"));
};


Wissel::Wissel(IO * rechtdoor, IO * afbuigend , int id)  {
  if (rechtdoor == NULL){
    Serial.print(F("rechtdoor = null, wissel ID = "));
    Serial.println(id);
    return;
  }
  if (afbuigend == NULL){
    Serial.print(F("afbuigend == NULL, wissel ID = "));
    Serial.println(id);
    return;
  }
  
  this->rechtdoor = rechtdoor;
  this->afbuigend = afbuigend;
  richting = Richting::onbekend;
  this->id = id;
};

void Wissel::init(void){
  this->rechtdoor->init(OUTPUT, 0);
  this->afbuigend->init(OUTPUT, 0);
}





void Wissel::activate(Richting r) {
  if (this->richting == r)   return;
  IO * pin;
  debug("wissel ")
  debug(id);
  if (r == Richting::afbuigend) {
    pin = afbuigend;

    debugln(": afbuigend");
  } else {
    pin = rechtdoor;
    debugln(": rechtdoor");
  }
  pin->setHigh();
  delay(125);
  pin->setLow();
  delay(25);
  this->richting = r;

}

void Wissel::zetafbuigend() {
  this->activate(Richting::afbuigend);
}

void Wissel::zetrechtdoor() {
  this->activate(Richting::rechtdoor);
}

Richting Wissel::getRichting(void) {
  return richting;
}
