#include "Button.h"

Button::Button(byte pin) {
  this->pin = pin;
  lastReading = LOW;
  last_state = LOW;
  init();
}

void Button::init() {
  pinMode(pin, INPUT);
  update();
}

void Button::update() {
    // You can handle the debounce of the button directly
    // in the class, so you don't have to think about it
    // elsewhere in your code
    byte newReading = digitalRead(pin);
    
    if (newReading != lastReading) {
      lastDebounceTime = millis();
    }

    if (millis() - lastDebounceTime > debounceDelay) {
      // Update the 'state' attribute only if debounce is checked
      state = newReading;
    }

    lastReading = newReading;
}

byte Button::getState() {
  last_state = state;
  update();
  if (last_state!=state){
      toggled=true;
  }
  else{
      toggled=false;
  }
  return state;
}

bool Button::isPressed() {
  return (getState() == HIGH);
}
bool Button::isToggled(byte high_low){
    bool ans = false;
    if (getState() == high_low){
        if (toggled){
            ans=true;
        }
    }
    return ans;
}