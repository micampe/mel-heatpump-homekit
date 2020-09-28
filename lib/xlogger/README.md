# xLogger - ESP8266 logger library. 
It can show logs and execute commands via telnet 

## Logger can do:

- put log to the serial port (hardware, software, stream). Can be enabled and disabled.
- save part of the log in RAM. (LOG_SIZE - length of memory buffer)
- put log to the connected terminal via the telnet port (23 by default)
- password can be settled for telnet connection
- filter the log by the log level: info /warning/error
- display the level in message list
- show the time in message list in intervals between lines, milliseconds, seconds and GMT time
- execute built-in and user's commands

## Sample initial telnet message with password and user commands

```*** Telnet debug for ESP8266.
Program version: 0.92 Logger version: 1.0.
Host name: XXXXXX IP:192.168.99.168 Mac address:XX:XX:XX:XX:XX:XX
Free Heap RAM: 30088

Command serial [enable|disable|?] write log to serial debug port.
Serial: enable
Command showdebuglvl [enable|disable|?] shows debug level in log lines.
Show debug level: enable
Command loglvl [info|warning|error|?] filters messages by log level.
Log level: Info
Command time [none|str|ms|btw|gmt|?] shows time in log lines.
Time format: gmt time (needs NTP available)
Command reboot reboots ESP.
Command startwificfg puts ESP to configure mode. Show configuration AP.
Command cfgdevice writes RS-485 device type to ESP memory and reboot.

Please, enter password before entering commands. Password length may be up to 10 symbols. 
```

## Logger functions

`xLogger()`

Constructor. Creates object.

`void begin(char *_hostName, Stream *_serial = NULL, bool _serialEnabled = false, char *_passwd = "")`

Initializes object

`void handle()`

This function nneds to call in loop cycle

`void cmdCallback(logCallback, const char* = NULL)`

Set callback to function that executes the user commands and command's help

`void setSerial(Stream *_serial)`

Set serial port that log lines put to. It can be: HardwareSerial, SoftwareSerial, Stream, Print, loggers

`void enableSerial(bool _serialEnabled)`

Enable/disable put log lines to serial port

`void setPassword(char *_passwd)`

Set password for access logger via telnet port

`void setProgramVersion(char * _programVersion)`

Set pointer to null-terminated string with program's version. It displays via telnet.

`void setTimeFormat(LogTimeFormat _timeFormat)`

Change time format for displaying in log lines

`void setShowDebugLevel(bool _showDebugLevel)`

Enable/disable showing debug level in log lines

`void setFilterDebugLevel(LogLevel _logLevel)`

Filters outting and showing log lines by their minimal debug level
