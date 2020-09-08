#include <Arduino.h>
#include <TelnetStream.h>

#define MICROSEC(s) (s * 1000000L)

void heartbeat(unsigned seconds, bool blink) {
  static unsigned long counter = micros();

  if (micros() > counter) {
    if (blink) {
      int led_bri = 10;
      int pwm = PWMRANGE - (int)(led_bri * 1.0 * PWMRANGE / 100.0 + 0.5f);
      analogWrite(LED_BUILTIN, pwm);
      // digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
    }

    Serial.print(".");
    TelnetStream.print(".");
    counter = micros() + MICROSEC(seconds);
  }
}
