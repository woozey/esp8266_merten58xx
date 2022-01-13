#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <connection_credentials.h>
#include <config.h>
#include <logger.h>

// TODO: webserver for configuration
// TODO: save configuration to EEPROM
// TODO: OTA
// TODO: check millis overflow
// TODO: AP if not connected?

// General:
const char *version = "0.1dev";
const int init_delay = 10000;  // delay before initialization
const int timeout = 20;   // connection timeout
Logger logger(LOG_LEVEL, "serial", 0);

// WIFI:
// int wifi_eep_addr = 0;
WiFiClient wclient;


// WEB:
int web_port = 80;
AsyncWebServer web_server(web_port);
String input3;

// MQTT:
// int mqtt_eep_addr = udp_eep_addr + 18 +1;


char mqtt_set_topic[24];
char mqtt_status_topic[24];
char mqtt_state_topic[24];

String mqtt_name = "vmespmic";
PubSubClient mqtt_client(wclient);

// Buttons
int button_up_state;
int button_down_state;

// States and motion
String motion_state = "idle"; 
unsigned long motion_start_time;
char *set_state = (char *)"stop";
char *current_state = (char *)"stop";
int move_up = 0;
int move_down = 0;

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP EEPROM");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
  <form action="/get" target="hidden-form">
    inputString (current value %inputString%): <input type="text" name="inputString">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    Sample length (current value %sample_length%): <input type="number " name="sample_length">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    Number of samples (current value %sample_number%): <input type="number " name="sample_number">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

void not_found(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    return "aaa";
  }
  else if(var == "sample_length"){
    // EEPROM.get(mic_eep_addr, mic_config);
    // return String(mic_config.sample_length);
    return "bbb";
  }
  else if(var == "sample_number"){
    // EEPROM.get(mic_eep_addr, mic_config);
    // return String(mic_config.sample_number);
    return "ccc";
  }
  return String();
}

void callback(char *topic, byte *payload, int length) {
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);
   Serial.print("Message:");
   set_state = (char*)payload;
   Serial.println();
}

String publish_mqtt(const char* data, PubSubClient client, const char* topic){
  client.publish(topic, data);
  Serial.println("Data sent to MQTT server");
  return "";
}

String print_serial_str(String data){
  Serial.print('\n');
  Serial.println(data);
  return "";
}

String print_serial_cha(const char* data){
  Serial.println(data);
  return "";
}

// char* array2msg(uint data[]){
//   char cdata = 
//   for (int i = 0; i < sample_length; i++){

//   }
// }

void start_wifi(const char *ssid, const char *pass){
  // WiFi.hostname("esp_mserten_01");
  WiFi.begin(ssid, pass);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  if (WIFI_MODE == (char *)"sta"){
    WiFi.mode(WIFI_STA);
    Serial.print(", mode STA");
  }
  Serial.println(" ...");
  int i = 0;
  while ((WiFi.status() != WL_CONNECTED) && (i < timeout)) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  if (WiFi.status() == WL_CONNECTED){
    Serial.println('\n');
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  }
  else
  {
    Serial.print('\n');
    Serial.println("Cannot connect to WIFI, timeout!");
  }
}

void start_mqtt(PubSubClient client, IPAddress mqtt_server, const int mqtt_port, const char *mqtt_user, const char *mqtt_pass){
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  int i = 0;
  Serial.print("Connecting to mqtt broker, attempt: ");
  while ((!client.connected()) && (i < timeout) && (WiFi.status() == WL_CONNECTED)) {
    // Serial.println('\n');
    Serial.print(++i); Serial.print(' ');
    client.connect("esp8266-client", mqtt_user, mqtt_pass);
    delay(1000);
  }
  Serial.print('\n');
  if (client.connected()) {
        Serial.println("Mqtt broker connected");
    } else {
        Serial.print("Connection timeout. Failed with state: ");
        Serial.println(client.state());
        }
}

void publish_mqtt_states(PubSubClient mqtt_client, char *set_state, char *current_state){
  if (mqtt_client.connected()){
    publish_mqtt(set_state, mqtt_client, mqtt_set_topic);
    publish_mqtt(current_state, mqtt_client, mqtt_state_topic);
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  delay(init_delay);

  logger.info("Starting esp8266_merten58xx version " + (String)version);
  
  // ===== WiFi =====
  EEPROM.begin(512);
  // EEPROM.put(wifi_eep_addr, WIFI_DATA);
  // EEPROM.commit();
  // EEPROM.get(wifi_eep_addr, WIFI_DATA);
  start_wifi(WIFI_DATA.ssid, WIFI_DATA.pass);

  // ===== Web =====
  // Send web page with input fields to client
  web_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  web_server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET sl value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam("sample_length")) {
      inputMessage = request->getParam("sample_length")->value();
      // mic_config.sample_length = inputMessage.toInt();
      // EEPROM.put(mic_eep_addr, mic_config);
      // EEPROM.commit();
    }
    // GET sn value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam("sample_number")) {
      inputMessage = request->getParam("sample_number")->value();
      // inputParam = PARAM_INPUT_2;
      // input2 = inputMessage;
      // mic_config.sample_number = inputMessage.toInt();
      // EEPROM.put(mic_eep_addr, mic_config);
      // EEPROM.commit();
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam("inputString")) {
      inputMessage = request->getParam("inputString")->value();
      inputParam = "inputString";
      input3 = inputMessage;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    // request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
    //                                  + inputParam + ") with value: " + inputMessage +
    //                                  "<br><a href=\"/\">Return to Home Page</a>");
    request->send(200, "text/text", inputMessage);
  });
  web_server.onNotFound(not_found);
  web_server.begin();

  // ===== MQTT ===== 
  // EEPROM.put(mqtt_eep_addr, MQTT_DATA);
  // EEPROM.commit();
  // EEPROM.get(mqtt_eep_addr, mqtt_data);
  MQTT_SET_TOPIC.toCharArray(mqtt_set_topic, 24);
  MQTT_STATUS_TOPIC.toCharArray(mqtt_status_topic, 24);
  MQTT_STATE_TOPIC.toCharArray(mqtt_state_topic, 24);

  IPAddress mqtt_server(MQTT_DATA.ip[0], MQTT_DATA.ip[1], MQTT_DATA.ip[2], MQTT_DATA.ip[3]);
  start_mqtt(mqtt_client, mqtt_server, MQTT_DATA.port, MQTT_DATA.user, MQTT_DATA.pass);
  if (mqtt_client.connected()){
    publish_mqtt("ready", mqtt_client, mqtt_status_topic);
    publish_mqtt(current_state, mqtt_client, mqtt_state_topic);
  }
  mqtt_client.subscribe(mqtt_set_topic);
}
// ======= LOOP ======
void loop() {
  button_up_state = digitalRead(BUTTON_UP);
  button_down_state = digitalRead(BUTTON_DOWN);
  
  // === Prepare MQTT motion trigers ===
  if (set_state != current_state){
    move_down = (set_state == (char *)"closed");
    move_up = (set_state == (char *)"open");
  }
  else{
    move_up = 0;
    move_down = 0;
  }

  //  === Motion states: idle or moving ===
  if (motion_state == "idle"){
    
    // Move UP
    if ((button_up_state == HIGH && !(current_state == (char *)"open")) || move_up){
      logger.debug("Start moving UP.");
      digitalWrite(RELAY_UP, HIGH);
      motion_state = "moving";
      motion_start_time = millis() * 1000;
      current_state = (char *)"open";
      set_state = (char *)"open";
    }
    
    // Move DOWN
    if ((button_down_state == HIGH && !(current_state == (char *)"closed")) || move_down){
      logger.debug("Start moving DOWN.");
      digitalWrite(RELAY_DOWN, HIGH);
      motion_state = "moving";
      motion_start_time = millis() * 1000;
      current_state = (char *)"closed";
      set_state = (char *)"closed";
    }
  }
  // Stop moving if button pressed or if sufficent time elapsed
  if (motion_state == "moving"){
    if ((motion_start_time + STOP_AFTER) >= millis()*1000 || button_up_state == HIGH || button_down_state == HIGH){
      logger.debug("Stop moving.");
      digitalWrite(RELAY_UP, LOW);
      digitalWrite(RELAY_DOWN, LOW);
      publish_mqtt_states(mqtt_client, set_state, current_state);
    }
  }

  if (LED_STATUS!=0){
    if (WiFi.status() != WL_CONNECTED){
      digitalWrite(LED_STATUS, HIGH);
      delay(200);
      digitalWrite(LED_STATUS, LOW);
      delay(200);
      digitalWrite(LED_STATUS, HIGH);
      delay(200);
      digitalWrite(LED_STATUS, LOW);
      delay(200);
      digitalWrite(LED_STATUS, HIGH);
      delay(200);
      digitalWrite(LED_STATUS, LOW);
    }
    else if (!mqtt_client.connected()){
      digitalWrite(LED_STATUS, HIGH);
      delay(100);
      digitalWrite(LED_STATUS, LOW);
      delay(100);
      digitalWrite(LED_STATUS, HIGH);
      delay(100);
      digitalWrite(LED_STATUS, LOW);
    }
  }
}
