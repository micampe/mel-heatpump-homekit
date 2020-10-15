#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiManager.h>

extern WiFiManager wifiManager;

void initWiFiManager(const char* ssid);

#endif
