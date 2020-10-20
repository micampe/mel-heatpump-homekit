#include "xlogger.h"

char pf_buffer[PRINTF_BUFFER_LENGTH];
char lineBuffer[LINE_BUFFER_LENGTH] = {0};
int lineBufferLen = 0;
time_t lastBootTime;

const char *strLogLevel[llLast] = {
  "n/a",
  "Info",
  "Warning",
  "Error",
};

const char *strLogTimeFormat[ltLast] = {
  "none",
  "time from start",
  "milliseconds from start",
  "milliseconds from previous log entry",
  "utc"
};

xLogger::xLogger() {
  
}

void xLogger::begin(const char _hostName[], Stream *_serial, bool _serialEnabled, const char _passwd[]) {
  lastBootTime = now();
  hostName = String(_hostName);
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  setSerial(_serial);
  enableSerial(_serialEnabled);
  setPassword(_passwd);
}

void xLogger::cmdCallback(logCallback cb, const char* _commandDescription) {
  _cmdCallback = cb;
  commandDescription = _commandDescription;
}

void xLogger::enableSerial(bool _serialEnabled) {
  serialEnabled = _serialEnabled && logSerial;
}

void xLogger::setProgramVersion(char * _programVersion) {
  programVersion = _programVersion;
}

void xLogger::handle() {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) // Close the last connect - only one supported
        telnetClient.stop();

      // Get telnet client
      telnetClient = telnetServer.available();
      telnetClient.flush();  // clear input buffer, else you get strange characters

      // clear authenticate
      telnetAuthenticated = !strnlen(passwd, 1);

      // Show the initial message
      showInitMessage();
      if (telnetAuthenticated)
        showLog();

      // Empty buffer in
      while(telnetClient.available()) 
          telnetClient.read();

    }
  }

  // Is client connected ? (to reduce overhead in active)
  telnetConnected = (telnetClient && telnetClient.connected());

  if (telnetConnected) {
    // get buffer from client
    while(telnetClient.available()) {  
      char c = telnetClient.read();

      if (c == '\n' || c == '\r') {
        
        // Process the command
        if (telnetCommand.length() > 0) 
          processCommand(telnetCommand);
          
        telnetCommand = ""; 
      } else 
        if (isPrintable(c)) 
          telnetCommand.concat(c);
    }
  }
  
}

bool xLogger::ExecCommand(const String &cmd) {
  if (cmd == "?") {
    showInitMessage();
    return true;
  }

  if (cmd == "showlog") {
    showLog();
    return true;
  }

  if (cmd == "mem") {
    println(SF("Free Heap: ") + String(ESP.getFreeHeap()));
    return true;
  }

  if (cmd == "uptime") {
    char str[20];
    uptimeString(str, 20, millis() / 1000);
    println(str);
    return true;
  }

  if (cmd == "serial enable") {
    serialEnabled = true;
    return true;
  }
  if (cmd == "serial disable") {
    serialEnabled = false;
    return true;
  }
  if (cmd == "serial ?") {
    println(SF("Serial: ") + (serialEnabled ? SF("enable") : SF("disable")));
    return true;
  }

  if (cmd == "showdebuglvl enable") {
    showDebugLevel = true;
    return true;
  }
  if (cmd == "showdebuglvl disable") {
    showDebugLevel = false;
    return true;
  }
  if (cmd == "showdebuglvl ?") {
    println(SF("Show debug level: ") + (showDebugLevel ? SF("enable") : SF("disable")));
    return true;
  }

  if (cmd == "loglvl info") {
    filterLogLevel = llInfo;
    return true;
  }
  if (cmd == "loglvl warning") {
    filterLogLevel = llWarning;
    return true;
  }
  if (cmd == "loglvl error") {
    filterLogLevel = llError;
    return true;
  }
  if (cmd == "loglvl ?") {
    println(filterLogLevel, "Log level: " + String(strLogLevel[filterLogLevel]));
    return true;
  }

  if (cmd == "time none") {
    logTimeFormat = ltNone;
    return true;
  }
  if (cmd == "time str") {
    logTimeFormat = ltStrTime;
    return true;
  }
  if (cmd == "time ms") {
    logTimeFormat = ltMsTime;
    return true;
  }
  if (cmd == "time btw") {
    logTimeFormat = ltMsBetween;
    return true;
  }
  if (cmd == "time utc") {
    logTimeFormat = ltUTCTime;
    return true;
  }
  if (cmd == "time ?") {
    println("Time format: " + String(strLogTimeFormat[logTimeFormat]));
    return true;
  }

  return false;
}

bool xLogger::processCommand(String &cmd) {
  // process login
  if (!telnetAuthenticated) {

    if (cmd == String(passwd)) {
      telnetClient.println("Password accepted.");
      println(llInfo, "Password accepted.");

      showLog();

      telnetAuthenticated = true;
      return true;
    }

    telnetClient.println("Password rejected.");
    println(llError, "Password (" + cmd + ") rejected.");

    return false;
  }

  // process command
  println(llInfo, "Telnet received command: " + cmd);

  if (ExecCommand(cmd)) {
    return true;
  }

  bool res = false;
  if (_cmdCallback) 
    res = _cmdCallback(cmd);

  if (!res) 
    println(llError, "Unknown command.");
  
  return res;
}


void xLogger::setSerial(Stream *_serial) {
  logSerial = _serial;
}

void xLogger::setPassword(const char *_passwd) {
  strncpy(passwd, _passwd, 10);
  telnetAuthenticated = !strnlen(passwd, 1);
}

void xLogger::setTimeFormat(LogTimeFormat _timeFormat) {
  logTimeFormat = _timeFormat;
}

void xLogger::setShowDebugLevel(bool _showDebugLevel) {
  showDebugLevel = _showDebugLevel;
} 

void xLogger::setFilterDebugLevel(LogLevel _logLevel) {
    filterLogLevel = _logLevel;
}

void xLogger::showInitMessage() {
  String msg = SF("*** Telnet debug for ESP8266.\r\n");

  if (programVersion && strnlen(programVersion, 1))
    msg += SF("Program version: ") + String(programVersion);
  msg += SF("\r\nHost: ") + hostName;
  msg += SF("\r\nIP  : ") + WiFi.localIP().toString();
  msg += SF("\r\nMac : ") + WiFi.macAddress();
  msg += SF("\r\nHeap: ") + String(ESP.getFreeHeap());

  char str[20];
  utcTimeToStr(str, 20, lastBootTime);
  msg += SF("\r\nBoot  : ") + str;
  utcTimeToStr(str, 20, now());
  msg += SF("\r\nTime  : ") + str;
  uptimeString(str, 20, millis() / 1000);
  msg += SF("\r\nUptime: ") + str;

  msg += SF("\r\n\r\nCommands:\r\n");
  msg += SF("serial [enable|disable] write log to serial debug port. [");
  msg += (serialEnabled ? SF("enabled]\r\n") : SF("disabled]\r\n"));
  msg += SF("showdebuglvl [enable|disable] shows debug level in log lines. [");
  msg += (showDebugLevel ? SF("enabled]\r\n") : SF("disabled]\r\n"));
  msg += SF("loglvl [info|warning|error] filters messages by log level. [");
  msg += String(strLogLevel[filterLogLevel]) + SF("]\r\n");
  msg += SF("time [none|str|ms|btw|utc] shows time in log lines. [");
  msg += String(strLogTimeFormat[logTimeFormat]) + SF("]\r\n");
  msg += SF("mem print free heap.\r\n");
  msg += SF("uptime print module uptime.\r\n");
  if (commandDescription && _cmdCallback)
    msg += String(commandDescription) + STR_RN;
  msg += STR_RN;

  if (!telnetAuthenticated && strnlen(passwd, 1))
    msg += SF("Please, enter password before entering commands. Password length may be up to 10 symbols.") + STR_RN;

  telnetClient.print(msg);
}

int xLogger::getNextLogPtr(int fromPtr) {
  int ptr = 0;
  LogHeader header;

  while (ptr <= LOG_SIZE - 1) {
    memcpy(&header, &logMem[ptr], sizeof(LogHeader));
    if (header.logTime == 0 && header.logLevel == llNone)
      return ptr;

    // check data length
    if (header.logSize < 0 || header.logSize > LOG_SIZE - ptr + 1)
      break;
      
    ptr += sizeof(LogHeader) + header.logSize + 1;
    if (ptr > fromPtr)
      return ptr;
  }
  
  return -1;
}

int xLogger::getEmptytLogPtr() {
  int ptr = 0;
  LogHeader header;

  while (ptr <= LOG_SIZE - 1) {
    memcpy(&header, &logMem[ptr], sizeof(LogHeader));
    if (header.logTime == 0 && header.logLevel == llNone) {
      return ptr;
    }

    // check data length
    if (header.logSize < 0 || header.logSize > LOG_SIZE - ptr + 1)
      break;

    ptr += sizeof(LogHeader) + header.logSize + 1;
  }

  return -1;
}

void xLogger::addLogToBuffer(LogHeader &header, const char *buffer) {
  if (header.logSize <= 0 || !buffer)
    return;

  int ptr = getEmptytLogPtr();

  // check buffer length
  if ((ptr < 0) || (ptr + sizeof(LogHeader) + header.logSize + 1 > LOG_SIZE)) {
    // get size that we need in buffer
    int qptr = getNextLogPtr(max(LOG_SEGMENT, (int)sizeof(LogHeader) + header.logSize + 1));
    if (qptr > 0) {
      // move buffer with 0x00 tail
      memmove(&logMem[0], &logMem[qptr], LOG_SIZE + sizeof(LogHeader) - qptr);
      ptr = getEmptytLogPtr();
    } else {
      // if we cant get next pointer
      return;   
    }
  }

  // double check
  if ((ptr < 0) || (ptr + sizeof(LogHeader) + header.logSize + 1 > LOG_SIZE)){ 
    return;
  }

  // copy header
  memcpy(&logMem[ptr], &header, sizeof(LogHeader));
  ptr += sizeof(LogHeader);

  // copy buffer
  memcpy(&logMem[ptr], buffer, header.logSize);
  ptr += header.logSize;
  
  // add 0x00 at the end
  logMem[ptr] = 0x00;  
  ptr++;
  
  // fill next log entry with 0x00
  memset(&logMem[ptr], 0x00, sizeof(LogHeader));
}

void xLogger::showLog() {
  telnetClient.println(SF("*** Cached log:"));

  int ptr = 0;
  String str;
  LogHeader header;

  while (ptr <= LOG_SIZE - 1) {
    memcpy(&header, &logMem[ptr], sizeof(LogHeader));
    if (header.logTime == 0 && header.logLevel == llNone) {
      break;
    }

    formatLogMessage(str, (char*)(&logMem[ptr] + sizeof(LogHeader)), header.logSize, &header);
    telnetClient.print(str);
    
    ptr += sizeof(LogHeader) + header.logSize + 1;
  }
  telnetClient.println(SF("***"));
}

void xLogger::formatLogMessage(String &str, const char *buffer, size_t size, LogHeader *header) {
  str = "";

  if (header) {
    // show time
    switch (logTimeFormat) {
      case ltStrTime:
        char uptime[20];
        uptimeString(uptime, 20, header->logTime / 1000);
        str = String(uptime);
        str += SF(" ");
        break;
      case ltMsTime:
        str = String(header->logTime);
        str += SF(" ");
        break;
      case ltMsBetween:
        str = String(header->logTime - oldMillis);
        str += SF(" ");
        oldMillis = header->logTime;
        break;
      case ltUTCTime:
        if (timeStatus() != timeNotSet) {
          char time[20];
          utcTimeToStr(time, 20, lastBootTime + header->logTime / 1000);
          str = String(time);
          str += SF(" ");
        } else {
          str = "[NTP sync error] ";
        }
        break;
      case ltNone:
      case ltLast:
        break;
    };
  
    // show log level
    if (showDebugLevel) {
      switch (header->logLevel) {
        case llInfo:     str += SF("INFO: "); break;
        case llWarning:  str += SF("WARNING: "); break;
        case llError:    str += SF("ERROR: "); break;
        default:         str += SF("UNKNOWN: "); break;
      }
    }
  }

  str += String(buffer);  
}

void xLogger::processLineBuffer() {
  // add end of char
  lineBuffer[lineBufferLen] = 0x00;

  // if end of line or buffer full
  if (lineBuffer[lineBufferLen - 1] != '\n' && LINE_BUFFER_LENGTH - 1 > lineBufferLen) {  
    return;
  }

  // processing...
  if (filterLogLevel <= curHeader.logLevel) { // filter here
    curHeader.logTime = millis();

    // write to buffer
    curHeader.logSize = lineBufferLen;
    addLogToBuffer(curHeader, &lineBuffer[0]);

    String msg = "";
    formatLogMessage(msg, lineBuffer, lineBufferLen, &curHeader);

    // write to serial
    if (serialEnabled && logSerial) {
      logSerial->print(msg);
    }

    // write to telnet
    if (telnetConnected && (telnetAuthenticated || !strnlen(passwd, 1)) ) { 
      telnetClient.print(msg);
    }
  }
  
  lineBufferLen = 0;
}

size_t xLogger::write(uint8_t c) {
  lineBuffer[lineBufferLen] = c;
  lineBufferLen++;

  processLineBuffer();
  return 1;
}

size_t xLogger::write(const uint8_t *buffer, size_t size) {
  if (!size)
    return size;
                              
  memcpy(&lineBuffer[lineBufferLen], buffer, min((int)size, LINE_BUFFER_LENGTH - lineBufferLen - 1)); // copy with checking length
  lineBufferLen += size;

  processLineBuffer();
  
  return size;
}

void uptimeString(char* str, size_t size, long val) {
    int days = elapsedDays(val);
    int hours = numberOfHours(val);
    int minutes = numberOfMinutes(val);
    int seconds = numberOfSeconds(val);

    if (days > 0) {
        snprintf(str, size, "%dd %dh %dm %ds", days, hours, minutes, seconds);
    } else if (hours > 0) {
        snprintf(str, size, "%dh %dm %ds", hours, minutes, seconds);
    } else if (minutes > 0) {
        snprintf(str, size, "%dm %ds", minutes, seconds);
    } else {
        snprintf(str, size, "%ds", seconds);
    }
}

void utcTimeToStr(char* str, size_t size, time_t time) {
  snprintf(str, size, "%04d-%02d-%02d %02d:%02d:%02d",
      year(time), month(time), day(time), hour(time), minute(time), second(time));
}
