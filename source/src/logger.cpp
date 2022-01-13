
#include <Arduino.h>
#include "logger.h"

Logger::Logger(String level, String log_to, int serial_baud) {
  if (level.equalsIgnoreCase("debug")){
      this->level = 1;
  }
  else if (level.equalsIgnoreCase("info")){
      this->level = 2;
  }
  else if (level.equalsIgnoreCase("warning")){
      this->level = 3;
  }
  else if (level.equalsIgnoreCase("error")){
      this->level = 4;
  }
  if (log_to.equalsIgnoreCase("serial")){
      this->log_to = 1;
      if (serial_baud){
          Serial.begin(serial_baud);
      }
  }
  else{
      error_0();
  }
}
void Logger::error_0() {
    Serial.println("ERROR: Logger initialization failed");
}

void Logger::print_log(String msg){
    if (log_to == 1){
        Serial.println(msg);
    }
}

void Logger::debug(String msg){
    if (level <= 1){
        print_log("DEBUG: " + msg);
    }
}

void Logger::info(String msg){
    if (level <= 2){
        print_log("INFO: " + msg);
    }
}

void Logger::warning(String msg){
    if (level <= 3){
        print_log("WARNING: " + msg);
    }
}

void Logger::error(String msg){
    if (level <= 4){
        print_log("ERROR: " + msg);
    }
}

void Logger::debug_w_int(String msg, int num){
    debug(msg + " " + num + ".");
}

void Logger::info_w_int(String msg, int num){
    info(msg + " " + num + ".");
}

void Logger::warning_w_int(String msg, int num){
    warning(msg + " " + num + ".");
}

void Logger::error_w_int(String msg, int num){
    error(msg + " " + num + ".");
}