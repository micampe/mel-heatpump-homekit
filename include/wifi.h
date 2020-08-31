#include <WiFiManager.h>
#include <ESP8266mDNS.h>

void setupWiFi(const char *nameprefix, const char *hostname) {
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);

  // reset settings - for testing
  // wifiManager.resetSettings();

  // sets timeout until configuration portal gets turned off
  // useful to make it all retry or go to sleep
  wifiManager.setTimeout(120);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  String ssid = nameprefix + String(ESP.getChipId(), HEX);
  if (!wifiManager.autoConnect(ssid.c_str())) {
    Serial.println(F("Failed to connect and hit timeout, restarting"));
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.print("Connected to WiFi: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(hostname)) {
    Serial.print(F("mDNS hostname: "));
    Serial.println(hostname);
  } else {
    Serial.println(F("mDNS failed to start"));
  }
}
