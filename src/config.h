
#define USE_MQTT

#ifdef USE_MQTT

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* mqttTopic          = "sonoff/s20/relay";
const char* mqttResetModeTopic = "sonoff/s20/mode/reset";

IPAddress broker(192,168,0,251);                     // Address of the MQTT broker

#define CLIENT_ID "client-000000"                    // Client ID to send to the broker

/* WiFi Settings */
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

#define USE_MQTT_AUTH

#define MQTT_USER "USERNAME"
#define MQTT_PASSWORD "PASSWORD"

#endif