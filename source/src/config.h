#include <Arduino.h>

// === Logger ===
String LOG_LEVEL  = "debug";

// === WiFi ===
const char *WIFI_MODE = (char *)"normal";

// === MQTT ===
String DEVICE_NAME = "blinds_living_room1";

String MQTT_STATE_TOPIC = DEVICE_NAME + "/current_state";
String MQTT_SET_TOPIC = DEVICE_NAME + "/set_point";
String MQTT_STATUS_TOPIC = DEVICE_NAME + "/status";

// === GPIO Settings ===
const int BUTTON_UP = 5;
const int BUTTON_DOWN = 12;
const int RELAY_UP = 2;
const int RELAY_DOWN = 16;
const int LED_STATUS = 0;

// === Motion ===
const unsigned long STOP_AFTER = 10; // time after the reley seconds 