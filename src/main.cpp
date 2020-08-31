#include <Arduino.h>

#include "OTA.h"
#include "heartbeat.h"
#include "wifi.h"

#define NAME_PREFIX "HKM"

void setup() {
  Serial.begin(115200);
  Serial.println();

  setupWiFi(NAME_PREFIX, "hkm");
  setupOTA(NAME_PREFIX);
}

void loop() {
  heartbeat(10);
  handleOTA();
}
