#include <Arduino.h>

#define STATUS_LED_PIN D4
#define MICROSEC(s) (s * 1000000L)

void heartbeat(unsigned seconds, bool blink) {
  static unsigned long counter = micros();

  if (micros() > counter) {
    if (blink) {
      int led_bri = 10;
      int pwm = PWMRANGE - (int)(led_bri * 1.0 * PWMRANGE / 100.0 + 0.5f);
      analogWrite(STATUS_LED_PIN, pwm);
      // digitalWrite(STATUS_LED_PIN, LOW);
      delay(50);
      digitalWrite(STATUS_LED_PIN, HIGH);
    }

    Serial.print(".");
    counter = micros() + MICROSEC(seconds);
  }
}
