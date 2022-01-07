#include <Arduino.h>

// === MQTT ===
char *DEVICE_NAME = (char *)"blinds_living_room1";

char *MQTT_STATE_TOPIC = strcat(DEVICE_NAME, "/current_state");
char *MQTT_SET_TOPIC = strcat(DEVICE_NAME, "/set_point");
char *MQTT_STATUS_TOPIC = strcat(DEVICE_NAME, "/status");

// === GPIO Settings ===
const int BUTTON_UP = 5;
const int BUTTON_DOWN = 6;
const int RELAY_UP = 7;
const int RELAY_DOWN = 8;
const int LED_STATUS = 9;

// === Motion ===
const unsigned long STOP_AFTER = 10; // time after the reley seconds 