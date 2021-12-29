#include <Arduino.h>

struct { 
    char ssid[20] = "your_WiFi_SSID";
    char pass[20] = "your_WIFI_pass";
  } WIFI_DATA;

struct { 
    uint16 port = 1234; // PUT MQTT SERVER PORT NUMBER HERE
    char pass[24] = "your_MQTT_pass";
    char user[16] = "your_MQTT_user";
    uint8 ip[4] = {256,256,256,256}; // PUT MQTT SERVER IP ADDRESS HERE
  } MQTT_DATA;
  