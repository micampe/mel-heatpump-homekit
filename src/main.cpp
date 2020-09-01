#include <Arduino.h>

#include <arduino_homekit_server.h>

#include "OTA.h"
#include "heartbeat.h"
#include "wifi.h"

#define NAME_PREFIX "HKM"

void homekit_setup();
void homekit_loop();

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  setupWiFi(NAME_PREFIX);
  setupOTA(NAME_PREFIX);
  homekit_setup();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  heartbeat(10, true);
  handleOTA();
  homekit_loop();
}

// HomeKit setup

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" void accessory_init();

void homekit_setup() {
    // homekit_storage_reset();
	accessory_init();
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

	arduino_homekit_setup(&config);
}

void homekit_loop() {
	arduino_homekit_loop();
	// static uint32_t next_heap_millis = 0;
	// uint32_t time = millis();
	// if (time > next_heap_millis) {
	// 	SIMPLE_INFO("heap: %d, sockets: %d", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
	// 	next_heap_millis = time + 5000;
	// }
}
