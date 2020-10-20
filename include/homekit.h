#ifndef HOMEKIT_H
#define HOMEKIT_H

#include <functional>

// audo mode doesn't report the fan speed, we set a default to have a
// consistent value so HomeKit can properly detect when scenes are active
#define AUTO_FAN_SPEED 1

#define HK_SPEED(s) ((float)s * 20)
#define HP_SPEED(s) (s <= 20 ? "QUIET" : s <= 40 ? "1" : s <= 60 ? "2" : s <= 80 ? "3" : "4")

void initHomeKitServer(const char *ssid, std::function<void()> loop);

#endif
