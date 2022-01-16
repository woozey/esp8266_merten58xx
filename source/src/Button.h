#ifndef MY_BUTTON_H
#define MY_BUTTON_H
#include <Arduino.h>
class Button {
  
  private:
    byte pin;
    byte state;
    byte lastReading;
    byte last_state;
    bool toggled;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
  public:
    Button(byte pin);
    void init();
    void update();
    byte getState();
    bool isPressed();
    bool isToggled(byte high_low);
};
#endif