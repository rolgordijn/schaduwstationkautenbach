#include "StopWatch.h"
#include <Arduino.h>

StopWatch::StopWatch() {

}

void StopWatch::startTimer(void) {
  startTime = micros();
  counter++;
}

void StopWatch::stopTimer(void) {
  duration = micros() - startTime;
  totalTime += duration;

}

float StopWatch::averageElapsedTimeMicros(void) {
  return 1.0 * totalTime / counter;
}

float StopWatch::averageElapsedTimeMillis(void) {
  return (0.001 * totalTime / counter);
}

unsigned long StopWatch::getCounter() {
  return counter;
}

void StopWatch::reset(void) {
  totalTime = 0;
  counter = 0;
}
