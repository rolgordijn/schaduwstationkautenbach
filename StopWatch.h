#pragma once


#include "IO.h"
class StopWatch {
    unsigned long startTime;
    unsigned long duration;
    unsigned long counter;
    unsigned long totalTime;
  public:
    StopWatch();

    void startTimer(void);

    void stopTimer(void);

    float averageElapsedTimeMicros(void);

    float averageElapsedTimeMillis(void);

    unsigned long getCounter();
    void reset(void);


};
