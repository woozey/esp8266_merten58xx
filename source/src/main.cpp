#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <connection_credentials.h>

// General:
const char *version = "0.1dev";
const int init_delay = 5000;  // delay before initialization
const int timeout = 10;   // connection timeout

// WIFI:
// int wifi_eep_addr = 0;
WiFiClient wclient;


// WEB:
int web_port = 80;
AsyncWebServer web_server(web_port);
String input3;

// MQTT:
// int mqtt_eep_addr = udp_eep_addr + 18 +1;

const char *mqtt_topic = "microphone_01";
// char *mic_topic;
// char *status_topic;
const char *mic_topic = "microphone_01/data";
const char *status_topic = "microphone_01/status";
String mqtt_name = "vmespmic";
PubSubClient client(wclient);

// Button
const int button_pin = 5;
int button_state = 0;

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
   for (int i = 0; i < length; i++) {
       Serial.print((char) payload[i]);
   }
   Serial.println();
   Serial.println("-----------------------");
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
  WiFi.begin(ssid, pass);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
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

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  delay(init_delay);

  Serial.print("Starting esp8266_merten58xx version ");
  Serial.println(version);
  
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
  IPAddress mqtt_server(MQTT_DATA.ip[0], MQTT_DATA.ip[1], MQTT_DATA.ip[2], MQTT_DATA.ip[3]);
  start_mqtt(client, mqtt_server, MQTT_DATA.port, MQTT_DATA.user, MQTT_DATA.pass);
}
// ======= LOOP ======
void loop() {
  button_state=digitalRead(button_pin);
  if (button_state == HIGH){
    Serial.print("Buuton ");
    Serial.print(button_pin);
    Serial.println(" is high.");
    if ((client.connected() && false) ){
      publish_mqtt("up", client, status_topic);
    }
  }
}
