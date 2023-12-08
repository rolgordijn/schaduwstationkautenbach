#pragma once
#include "IO.h"

enum class Richting  : uint8_t {
  onbekend = 0,
  rechtdoor = 1,
  afbuigend = 2
};

class Wissel {
  private:
    IO * afbuigend;
    IO * rechtdoor;
    Richting richting;
    int id; 
  public:

    Wissel(IO * rechtdoor, IO * afbuigend , int id);
    Wissel();
    void init(void);
    void activate(Richting r) ;
    void zetafbuigend();
    void zetrechtdoor() ;
    void off();
    Richting getRichting(void);
};
