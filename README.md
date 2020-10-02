# Mitsubishi heat pump HomeKit accessory

ESP8266 HomeKit accessory firmware for Mitsubishi heat pumps.

## Hardware

This uses the [SwiCago/HeatPump](https://github.com/SwiCago/HeatPump) library to communicate with the heat pump, so you should follow instructions there for all the details and for supported models (essentially all Mitsubishi models with the right CN105 connector appear to be supported).

Notes:
- if you plan to use a Wemos D1 mini board, you should **use version 1** (the one with the metal FCC shield). v3 appears to have a different serial configuration that requires a hardware modification to work, v1 can be wired directly with only a connector.

## Setup
After flashing the firmware the ESP8266 will create a WiFi netwoek named `MIE Heat Pump XXXXXX`, connect to it and set up your WiFi name and password.

After setting up the WiFi the LED on the module will briefly blink at 3Hz and then at 1Hz. Now the module is ready for pairing and you can use the Home app to add the accessory (password `111-11-111`).

## Usage
The heat pump will appear in Home as three accessory grouped together: a thermostat, a dehumidifier, and a fan and the three will coordinate to show the current status of the heat pump.

For example if the heat pump is in “cooling” mode, the thermostat will show cool, the fan will be running at the speed you set and the dehumidifier will be off.

The fan supports five speeds from 20 to 100%: 20% is the heat pump quiet setting and the rest are speeds 1 to 4.

If you set the fan to auto mode the auto button will be on and the speed will be set to 20% (this is just the HomeKit display, the heat pump fan will still be automatic). We need to set a fixed speed in auto so that HomeKit can detect when scenes are active; to make that work remember to set the fan speed to 20% if you enable auto mode when creating your scenes.

If you want to switch back to manual speed control you first have to turn off auto mode.

Both the fan and dehumidifier show a swing button: the fan controls the vertical vane swing and the dehumidifier controls the horizontal vane swing. When swing is disabled the vertical vane is set to auto and the horizontal vane is set to center.

## Limitations
Vane manual positioning is not supported: I couldn't find a way to do it in HomeKit. Slats exist but I couldn't figure out how to set them up. Ideas welcome, but first try setting it up in the [KomeKit Accessory Simulator](https://developer.apple.com/documentation/homekit/testing_your_app_with_the_homekit_accessory_simulator).

The dehumidifier reports a current humidity of 0%. HomeKit doesn't allow removing this, I plan on adding an humidity sensor but for now the only workaround is to turn off the “Include in Home Status” switch for the dehumidifier.

Lastly, **important**: the heat pump is *very slow* to respond to commands and slow to send back updates, so don't play around too much with settings: set up what you want and then let it update, give it up to 30 seconds to catch up, especially when switching modes or turning on and off.

Ideally you will set up scenes for the configurations you want instead of changing them manually in the accessories. I’m trying to come up with solutions to mitigate the problem on the UI presentation but the heat pump will always be slow.

## Debugging
If something is not working as expected you can connect to the module with `telnet` and see a log of the recent events and operations. Please include this log and a detailed explanation of what you did, what you expected to happen, and what actually happened. If any of this information is missing I will not be able to help.

## TODO
- Humidity sensor integration
- Remote temperature support (the heat pumps support receiving a remote room temperature for the thermostat)
- “Child lock” setting to disable changes from the IR remote
- OTA update from a web page running on the module
- Elgato history support
