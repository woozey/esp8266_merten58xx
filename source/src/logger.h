#ifndef MY_LOGGER_H
#define MY_LOGGER_H

#include <Arduino.h>

class Logger {
  
  private:
    int level;
    int log_to;
    void error_0();
    void print_log(String msg);
    
  public:
    Logger(String level, String log_to, int serial_baud);
    void debug(String msg);
    void info(String msg);
    void warning(String msg);
    void error(String msg);
    void debug_w_int(String msg, int num);
    void info_w_int(String msg, int num);
    void warning_w_int(String msg, int num);
    void error_w_int(String msg, int num);
};

#endif