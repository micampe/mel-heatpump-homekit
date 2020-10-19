#include "web.h"

#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Time.h>

#include "debug.h"

// CLI update:
// curl -F "firmware=@<FILENAME>.bin" <ADDRESS>/_update

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer updateServer;

extern const char* index_html;

void uptimeString(String &str) {
    long val = millis() / 1000;
    int days = elapsedDays(val);
    int hours = numberOfHours(val);
    int minutes = numberOfMinutes(val);
    int seconds = numberOfSeconds(val);

    str = "";
    if (days > 0) {
        str = String(days) + "d ";  
    }
    if (hours > 0) {
        str += String(hours) + "h ";  
    }
    if (minutes > 0) {
        str += String(minutes) + "m ";
    }
    str += String(seconds) + "s";
}

void initWeb() {
    updateServer.setup(&httpServer, "/_update");

    httpServer.on("/", HTTP_GET, []() {
        String uptime;
        uptimeString(uptime);

        String html = String(index_html);
        html.replace("__TITLE__", WiFi.hostname());
        html.replace("__UPTIME__", uptime);
        html.replace("__HEAP__", String(ESP.getFreeHeap()));
        html.replace("__FIRMWARE_VERSION__", GIT_DESCRIBE);

        httpServer.send(200, "text/html", html);
    });

    httpServer.on("/_reboot", HTTP_POST, []() {
        MIE_LOG("Reboot from web UI...");
        httpServer.send(200, "text/html", "Rebooting...");
        delay(1000);
        ESP.restart();
    });

    // homekit already initialized MDNS
    MDNS.addService("http", "tcp", 80);
    httpServer.begin();
}
