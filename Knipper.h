#pragma 
#include "IO.h"

class Knipper {
    private:
    int period; 
    int treshold; 

    public:
    Knipper();
    Knipper(int treshold, int period);
    bool getValue(void);
};


/*
 * 
 * #ifndef KNIpper_h
#define KNIpper_h

#include "Arduino.h"

class Knipper
{
  public:
    Knipper();
    Knipper(int treshold, int period);
    bool getValue(void);
  private:
    int treshold;
    int period;
    static bool flag;
    static void toggleFlag();
};

#endif

 */
