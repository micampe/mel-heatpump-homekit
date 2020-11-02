#include "wifi_manager.h"

#include <DoubleResetDetect.h>
#include <Ticker.h>

#include "debug.h"
#include "homekit.h"
#include "led_status_patterns.h"

WiFiManager wifiManager;

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
static DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

static void wifi_did_enter_config_mode(WiFiManager *wifiManager) {
    led_status_set(&status_led_waiting_wifi);
}

void wifi_init(const char* ssid) {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    wifiManager.setAPCallback(wifi_did_enter_config_mode);
    wifiManager.setConnectTimeout(30);
    wifiManager.setSaveConnectTimeout(30);
    wifiManager.setConfigPortalTimeout(120);

    if (drd.detect()) {
        led_status_signal(&status_led_double_reset);
        while (!wifiManager.startConfigPortal(ssid)) {
            MIE_LOG("WiFi config portail timed out, restarting");
            delay(1000);
            ESP.restart();
        }
    } else {
        while (!wifiManager.autoConnect(ssid)) {
            MIE_LOG("WiFi connection failed, trying again");
        }
    }

    WiFi.mode(WIFI_STA);
    MIE_LOG("WiFi Connection successful");
    MIE_LOG("IP: %s", WiFi.localIP().toString().c_str());
}
