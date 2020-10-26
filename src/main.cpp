#include <Arduino.h>
#include <Ticker.h>
#include <arduino_homekit_server.h>

#include "debug.h"
#include "env_sensor.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "led_status_patterns.h"
#include "mqtt.h"
#include "ntp_clock.h"
#include "web.h"
#include "wifi_manager.h"

#define NAME_PREFIX "Heat Pump "
#define HOSTNAME_PREFIX "heat-pump-"

char name[25];
char hostname[25];

void setup() {
    Serial.begin(115200);
    Serial.println();

    led_status_init(LED_BUILTIN, false);

    sprintf(name, NAME_PREFIX "%06x", ESP.getChipId());
    sprintf(hostname, HOSTNAME_PREFIX "%06x", ESP.getChipId());

    wifi_init(name);
    ntp_clock_init();
    debug_init(name);
    web_init(hostname);
    mqtt_init(name);
    env_sensor_init();

    homekit_init(name, loop);

    if (!heatpump_init()) {
        led_status_signal(&status_led_error);
    }

    led_status_done();
}

void loop() {
    httpServer.handleClient();
    arduino_homekit_loop();
    mqtt.loop();
    Debug.handle();
}
