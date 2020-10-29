# Mitsubishi heat pump HomeKit accessory

ESP8266 HomeKit accessory firmware for Mitsubishi heat pumps.

## Hardware

Most ESP8266 boards should work, but I only tested this with a D1 mini clone.
The version 1 boards (the ones with the metal shield) work more reliably and
can be connected to the heat pump with no additional components. The [wiring
diagram](images/wiring_base.png) is in the images directory; note that RX/TX
are crossed.

D1 mini v3 boards (the ones with the visible chips) have been source of issues
and require a hardware modification, so my recommendation is to avoid v3, use
v1 or other boards.

A wiring diagram using an ESP-01 board is available on the
[SwiCago/HeatPump](https://github.com/SwiCago/HeatPump) repository.

A cable with the correct connector is available for purchase
[here](http://www.usastore.revolectrix.com/Products_2/Cellpro-4s-Charge-Adapters_2/Cellpro-JST-PA-Battery-Pigtail-10-5-Position).
Ignore the colors of the wires, black will stay not connected.

## Setup

After flashing the firmware the ESP8266 will create a WiFi network named `Heat
Pump XXXXXX`. Connect to it and set up your WiFi name and password. The device
will then reboot and it will be ready to pair using the Home app. The pairing
code is `111-11-1111`.

After the first flashing and WiFi setup you can update the firmware from the
web interface.

## Usage

The heat pump will appear in Home as three accessories grouped together: a
thermostat, a dehumidifier, and a fan. The three components will coordinate to
show the current status of the heat pump.

The fan supports five speeds from 20 to 100%: 20% is the heat pump quiet
setting and the rest are speeds 1 to 4.

If you set up scenes with the fan in auto mode you should set the speed to 20%.
The speed setting is ignored when auto mode is on, but HomeKit uses it to
detect when a scene is active.

To use manual fan speed control you first have to turn off auto mode.

Both the fan and dehumidifier show a swing button: the fan controls the
vertical vane swing and the dehumidifier controls the horizontal vane swing.
When swing is disabled the vertical vane is set to auto and the horizontal vane
is set to center.

## Limitations

Vane manual positioning is not supported: I couldn't find a satisfactory way to
do it in HomeKit. 

The dehumidifier always reports 0% humidity. HomeKit doesn't allow removing
this, as a workaround you can turn off the “Include in Home Status” switch for
the dehumidifier. See below for a better solution.

Lastly, the heat pump is *very slow* to respond to commands and slow to send
back updates. Set up what you want and then let it update, give it up to 30
seconds or more to catch up, especially when turning on or off.

## Advanced optional features

### Humidity sensor

The firmware supports connecting a humidity sensor to fill in the missing
information from the heat pump. BME280 and DHT22 sensors are supported. BME280
must be at I2C address 0x76 and DHT22 data pin must be connected to GPIO2. The
[full wiring diagram](images/wiring_full.png) is in the images directory. Both
sensors are shown but you only need one.

Even with an external sensor the temperature visible in Home will be the one
provided by the heat pump, because that is the one it will use in its
thermostat.

The ESP produces significant heat so make sure to keep the sensor away from it.

### Web interface

The device exposes a web interface accessible using its IP address (that you
can find from your router/access point), or at `http://heat-pump-XXXXXX.local`
if your network supports mDNS/Bonjour (all Macs and many Linux systems do).

The web interface shows the current state of the device and allows firmware
upgrades and changing settings. Settings are only interesting if an external
temperature and humidity sensor is connected and allow publishing the sensor
readings over mqtt.

### Logging

To access operating logs connect to the ESP using telnet. The device maintains
a buffer of recent operations, so you can connect and see what it did even
after the fact.
