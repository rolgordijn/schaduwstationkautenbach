#pragma once

#include "MCP23017.h"


class IO {
  protected:
    uint8_t pin;
    bool tris;
    bool pinState;
    bool changed;
  public:
    IO();
    void setHigh();
    void setLow();
    virtual void init(int dir, int level) = 0;
    virtual bool getValue() = 0;
    virtual void setValue(bool val) = 0;
    virtual void setInput()  = 0;
    virtual void setOutput()  = 0;
    virtual void setPinMode(int dir) =   0;
    virtual int getPinMode (void) = 0;
    void clearChangedFlag();
    bool didChange(void);
};

class BasicIO : public IO {
  public:
    BasicIO();
    BasicIO(int pin, int tris, int val);
    BasicIO(int pin, int tris);
    void init(int dir, int level);
    void setHigh(void);
    void setLow(void);
    bool getValue(void);
    void setValue(bool val);
    void setInput();
    void setOutput();
    void setPinMode(int dir);
    int getPinMode(void);

};

class MCP23017IO : public IO {
  private:
    MCP23017 * ic;
    bool pinState;
  public:
    MCP23017IO();
    MCP23017IO(MCP23017 * ic, uint8_t pin);
    void init(int dir, int level);
    void setHigh(void);
    void setLow(void);
    bool getValue();
    void setValue(bool val);
    void setInput();
    void setOutput();
    void setPinMode(int dir);
    int getPinMode(void);

};
