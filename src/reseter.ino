#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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

#define RELAY 12
#define GREEN_LED 13
#define BUTTON 0

WiFiClient wificlient;
PubSubClient client(wificlient);

int _lastPress = millis();

int _relayState = 0;
int _resetMode = 0;
int _durationBetween = 10000;
int _isLocked = 0;
int _lockedUntil = 0;

void setup()
{
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(0), buttonPressed, CHANGE);

  Serial.begin(115200);
  
  digitalWrite(GREEN_LED, HIGH); // Turn the LED off by making the voltage HIGH

  WiFi.mode(WIFI_STA);
  
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  reconnectWifi();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* Prepare MQTT client */
  client.setServer(broker, 1883);
  client.setCallback(mqttMessage);
}

void buttonPressed()
{
  int input = digitalRead(0);

  if(input == 0)
  {
    _durationBetween = millis() - _lastPress;
    _lastPress = millis();
  }
  else
  {
    int durationPressed = millis() - _lastPress;
    if (durationPressed > 1000)
    {
      // long press
      setResetMode(1);
   }
    else if(_durationBetween < 500)
    {
      // double
      setResetMode(0);
    }
    else
    {
      // single
      setRelay(!_relayState, 1);
    }
    
  }
}

void setResetMode(int state)
{
  _resetMode = state;
  digitalWrite(GREEN_LED, !state);

  if(state == 1)
  {
    setRelay(1, 1);
  }
}

void setRelay(int state, int publish)
{
  if(_isLocked)
  {
    return;
  }

  _relayState = state;
  digitalWrite(RELAY, state);

  if(publish == 0)
  {
    return;
  }

  char mqttPubTopic[128];
  sprintf(mqttPubTopic, "%s%s", mqttTopic, "/state");
  if(state == 1)
  {
    Serial.println("ON");
    client.publish(mqttPubTopic, "ON");
  }
  else
  { 
    Serial.println("OFF");
    client.publish(mqttPubTopic, "OFF");
  }
}

void loop()
{
  if(_resetMode == 1 && _isLocked == 0)
  {
    performReset();
    lockRelayFor(20000);
  }

  if(_isLocked && _lockedUntil < millis())
  {
    _isLocked = 0;
    setResetMode(0);
  }
  
  if (WiFi.status() != WL_CONNECTED)
  {
    reconnectWifi();
  }
  else if (!client.connected())
  {
    reconnectMQTT();
  }
  else  
  {
    client.loop();
  }
}

void performReset()
{
  Serial.println("Perform reset...");
  for(int i = 0; i < 6; i++)
  {
    setRelay(0, 0);
    delay(500);
    setRelay(1, 0);
    delay(500);
  }

  Serial.println("Reset done");
}

void lockRelayFor(int duration)
{
  _isLocked = 1;
  _lockedUntil = millis() + duration;
}

void reconnectWifi()
{
  int counter = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    // blinking LED while connecting...
    if(counter++ % 2 == 0)
    {
      digitalWrite(GREEN_LED, HIGH);
    }
    else
    {
      digitalWrite(GREEN_LED,LOW);
    }
      
    // also putting a . on the serial for debugging
    Serial.print(".");

    if(counter > 30)
    { 
      // to many connecting attempts.. try to restart
      Serial.println("");
      Serial.println("Restarting...");
      ESP.restart();
    }
    
    delay(500);
  }

  setResetMode(_resetMode);
}

void reconnectMQTT() 
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting to connect to MQTT broker...");
    // Attempt to connect
#ifdef USE_MQTT_AUTH
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
#else
    if(cleint.connect(CLIENT_ID))
#endif
    {
      Serial.println("connected to MQTT broker");
      client.subscribe(mqttTopic);
      client.subscribe(mqttResetModeTopic);
    } 
    else 
    {
      Serial.print(" Reconnect failed. State=");
      Serial.println(client.state());
      Serial.println("Retry in 3 seconds...");
      return;
    }
  }
}

void mqttMessage(char* topic, byte* payload, unsigned int length) 
{
  if (!strcmp(topic, mqttTopic)) 
  {
    setRelayViaMqtt(payload, length);
  }
  else
  {
    setResetMode(1);
  }
}

int setRelayViaMqtt(byte* payload, unsigned int length)
{
    if (!strncasecmp_P((char *)payload, "OFF", length)) 
    {
      setRelay(0, 1);
    }
    else if (!strncasecmp_P((char *)payload, "ON", length)) 
    {
      setRelay(1, 1);
    }
    else if (!strncasecmp_P((char *)payload, "TOGGLE", length))
    {
      setRelay(!_relayState, 1);
    }
}