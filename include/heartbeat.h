#include <Arduino.h>

#define MICROSEC(s) (s * 1000000L)

void heartbeat(unsigned seconds) {
  static unsigned long counter = micros();

  if (micros() > counter) {
    Serial.print(".");
    counter = micros() + MICROSEC(seconds);
  }
}