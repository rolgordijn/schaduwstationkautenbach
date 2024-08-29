#pragma once

#include "MCP23017.h"
#include "IODebugMessage.h"

#include <String.h>


enum class Event {
  RISING_EDGE,
  FALLING_EDGE
};

class IO {

protected:
  uint8_t pin;
  bool tris;
  bool pinState;
  bool changed;

  int index;

  virtual bool readPinState() = 0;
  virtual void setPinState(bool level) = 0;
  IODebugMessage* debugMsg;

  void (*ioEventHandler)(int, Event);


  void handleStateChange(bool currentPinState);
  void triggerEvent(bool currentPinState);
  void updateState(bool currentPinState);
  void updateDebugMessage(bool currentPinState);

public:


  IO();
  void setHigh();
  void setLow();

  void setIndex(int index);
  int getIndex(void);



  int getValue(void);
  void setValue(bool val);
  virtual void setInput() = 0;
  virtual void setOutput() = 0;
  void setPinMode(int dir);
  virtual int getPinMode(void) = 0;
  void clearChangedFlag();
  bool didChange(void);

  void init(int dir, int level);
  void printDebugMsg(Print& printer = Serial);
  void setDebugMessage(IODebugMessage* debugMsg);


  void setCallback(void (*ioEventHandler)(int, Event));
};

class BasicIO : public IO {
private:

public:
  BasicIO();
  BasicIO(int pin, int tris, int val);
  BasicIO(int pin, int tris);

  void setInput();
  void setOutput();
  int getPinMode(void);

  bool readPinState();
  void setPinState(bool level);
};

class MCP23017IO : public IO {
private:
  MCP23017 ic;
  bool pinState;

public:
  MCP23017IO();
  MCP23017IO(MCP23017& ic, uint8_t pin);

  void setPinState(bool level);
  bool readPinState();

  void setInput();
  void setOutput();
  int getPinMode(void);
};
