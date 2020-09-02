#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <arduino_homekit_server.h>

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t accessory_name;
extern "C" void accessory_init();

#endif
