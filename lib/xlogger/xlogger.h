/*
 * xLogger library
 *
 * Logging to Serial port and/or Telnet. Saves last log lines to memory buffer (LOG_SIZE - size of this buffer).
 * Can execute user commands (reboot, start wifi config, etc...)
 * 
 * (c) Oleg Moiseenko 2017
 */

#ifndef __XLOGGER_H__
#define __XLOGGER_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>     // https://github.com/esp8266/Arduino
#include <TimeLib.h>         // https://github.com/PaulStoffregen/Time 

#define XLOGGER_VERSION      "1.0"

#define TELNET_PORT          23                  // telent port for remote connection
#define LOG_SIZE             4096                // size of log memory buffer in bytes
#define LOG_SEGMENT          256                 // minimal size of log rotating
#define PRINTF_BUFFER_LENGTH 128                 // buffer length for printf execution
#define LINE_BUFFER_LENGTH   256                 // buffer length for commands (concatinate print and println)
extern char pf_buffer[PRINTF_BUFFER_LENGTH];

// string to flash
#define SF(x) String(F(x))
#define STR_RN SF("\r\n")
// time conversion
void eTimeToStr(String &str, long val, bool fullPrint = false);

typedef bool (*logCallback)(String &cmd);

enum LogLevel: uint8_t{
  llNone,
  llInfo, 
  llWarning,
  llError,

  llLast
};
extern const char *strLogLevel[llLast];

enum LogTimeFormat: uint8_t {
  ltNone,
  ltStrTime,
  ltMsTime,
  ltMsBetween,

  ltLast
};
extern const char *strLogTimeFormat[ltLast];

struct LogHeader {
  int logTime = 0;
  uint16_t logSize = 0;
  LogLevel logLevel = llInfo;
};
struct LogEntity {
  LogHeader header;
  char data[];
};

class xLogger: public Print{
public:
  xLogger();

  void begin(const char _hostName[], Stream *_serial = NULL, bool _serialEnabled = false, const char _passwd[] = "");
  void handle();
  void cmdCallback(logCallback, const char* = NULL);

  bool ExecCommand(const String &cmd);

  void setSerial(Stream *_serial);
  void enableSerial(bool _serialEnabled);
  void setPassword(const char *_passwd);
  void setProgramVersion(char * _programVersion);
  void setTimeFormat(LogTimeFormat _timeFormat);
  void setShowDebugLevel(bool _showDebugLevel);
  void setFilterDebugLevel(LogLevel _logLevel);

  virtual size_t write(uint8_t c);
  virtual size_t write(const uint8_t *buffer, size_t size);

  template<typename... Args>
  void printf(LogLevel loglev, const char* fmtstr, Args... args) {
    curHeader.logLevel = loglev;
    snprintf(pf_buffer, sizeof(pf_buffer), fmtstr, args...);
    print(curHeader.logLevel, pf_buffer);
  }
  template<typename... Args>
  void printf(const char* fmtstr, Args... args) {
    printf(llInfo, fmtstr, args...);
  }
    
  template<typename... Args>
  void print(LogLevel loglev, Args... args) {
    curHeader.logLevel = loglev;
    Print::print(args...);
  }
  template<typename... Args>
  void print(Args... args) {
    print(llInfo, args...);
  }

  template<typename... Args>
  void println(LogLevel loglev, Args... args) {
    curHeader.logLevel = loglev;
    Print::println(args...);
  }
  template<typename... Args>
  void println(Args... args) {
    println(llInfo, args...);
  }
private:
  String hostName = "n/a";
  bool serialEnabled = false;
  Stream *logSerial = NULL;
  uint8_t logMem[LOG_SIZE + sizeof(LogHeader) + 8] = {0}; // 8 - guard interval
  char passwd[11] = {0};
  bool telnetConnected = false;
  char * programVersion = NULL;
  const char * commandDescription = NULL;
  bool showDebugLevel = true;
  LogLevel filterLogLevel = llInfo;
  int oldMillis = 0;
  LogTimeFormat logTimeFormat = ltStrTime;
  String telnetCommand = "";
  bool telnetAuthenticated = false;

  // command callback
  logCallback _cmdCallback;

  WiFiServer telnetServer = WiFiServer(TELNET_PORT);
  WiFiClient telnetClient;

  LogHeader curHeader;

  void showInitMessage();
    
  int getNextLogPtr(int fromPtr);
  int getEmptytLogPtr();
  void addLogToBuffer(LogHeader &header, const char *buffer);
  void showLog();
  void formatLogMessage(String &str, const char *buffer, size_t size, LogHeader *header);

  void processLineBuffer();
  bool processCommand(String &cmd);
};

#endif // ifndef __XLOGGER_H__

