#pragma once

#include "MCP23017.h"

#include <String.h> 


class IO {

protected:
  uint8_t pin;
  bool tris;
  bool pinState;
  bool changed;

  bool invertedLogic;
  
  int index;
  String name;



  virtual bool readPinState() const = 0;
  virtual void setPinState(bool level) = 0;

public:


  IO();
  [[deprecated("Use turnOn() instead")]]
  void setHigh();
  [[deprecated("Use turnOff() instead")]]
  void setLow();
  void turnOn();
  void turnOff();
  int getValue(void);
  void setValue(bool val);
  virtual void setInput() = 0;
  virtual void setOutput() = 0;
  void setPinMode(int dir);
  virtual int getPinMode(void) = 0;
  void clearChangedFlag();
  bool didChange(void);
  bool isInvertedLogic();
  void init(int dir, int level);
  void setInvertedLogic(bool invert);
  void setNameAndIndex(String pinName, int pinIndex);
  void debug();
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

  bool readPinState() const override;
  void setPinState(bool level) override;
};

class MCP23017IO : public IO {
private:
  MCP23017* ic;
  bool pinState;
  
public:
  MCP23017IO();
  MCP23017IO(MCP23017* ic, uint8_t pin);

  void setPinState(bool level) override;
  bool readPinState() const override;

  void setInput();
  void setOutput();
  int getPinMode(void);
};
