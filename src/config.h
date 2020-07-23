
const char* mqttTopic          = "sonoff/s20/relay";
const char* mqttResetModeTopic = "sonoff/s20/mode/reset";

IPAddress broker(192,168,0,251);                     // Address of the MQTT broker

#define CLIENT_ID "client-000000"                    // Client ID to send to the broker

/* WiFi Settings */
const char* ssid     = "YOUR_WLAN_SSID";
const char* password = "YOUR_WLAN_PASSWORD";

#define USE_MQTT_AUTH
#define MQTT_USER "MQTT_USER"
#define MQTT_PASSWORD "MQTT_PASSWORD"
