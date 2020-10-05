#include "config.h"

#define RELAY 12
#define GREEN_LED 13
#define BUTTON 0

#ifdef USE_MQTT
WiFiClient wificlient;
PubSubClient client(wificlient);
#endif

int _lastPress = millis();

int _relayState = 0;
int _resetMode = 0;
int _durationBetween = 10000;
int _isLocked = 0;
int _lockedUntil = 0;

int _lockDuration = 20000;
int _cycles = 6;
int _delayOn = 500;
int _delayOff = 500;

void setup()
{
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(0), buttonPressed, CHANGE);

  Serial.begin(115200);
  
  digitalWrite(GREEN_LED, HIGH); // Turn the LED off by making the voltage HIGH
#ifdef USE_MQTT
  WiFi.mode(WIFI_STA);
  
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  reconnectWifi();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* Prepare MQTT client */
  client.setServer(broker, 1883);
  client.setCallback(mqttMessage);
#endif
}

ICACHE_RAM_ATTR void buttonPressed()
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

  if(state == 1 && _relayState  == 0)
  {
    setRelay(1, 0);
    delay(2000);
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

  publishState();
}

void publishState()
{
#ifdef USE_MQTT
  char mqttPubTopic[128];
  sprintf(mqttPubTopic, "%s%s", mqttTopic, "/state");
  if(_isLocked==1)
  {
    client.publish(mqttPubTopic, "{\"state\": \"LOCKED\"}");
  }
  else if(_relayState == 1)
  {
    client.publish(mqttPubTopic, "{\"state\": \"ON\"}");
  }
  else
  { 
    client.publish(mqttPubTopic, "{\"state\": \"OFF\"}");
  }
#endif
}

void loop()
{
  if(_resetMode == 1 && _isLocked == 0)
  {
    performReset();
    lockRelayFor(_lockDuration);
  }

  if(_isLocked && _lockedUntil < millis())
  {
    _isLocked = 0;

    publishState();
    setResetMode(0);
  }
#ifdef USE_MQTT
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
#endif
}

void performReset()
{
  Serial.println("Perform reset...");
  for(int i = 0; i < _cycles; i++)
  {
    setRelay(0, 0);
    delay(_delayOff);
    setRelay(1, 0);
    delay(_delayOn);
  }

  Serial.println("Reset done");
}

void lockRelayFor(int duration)
{
  _isLocked = 1;
  _lockedUntil = millis() + duration;
  publishState();
}

#ifdef USE_MQTT
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
    if(client.connect(CLIENT_ID))
#endif
    {
      Serial.println("connected to MQTT broker");
      client.subscribe(mqttTopic);
      client.subscribe(mqttResetModeTopic);
      publishState();
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
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (!strcmp(topic, mqttTopic)) 
  {
    // Test if parsing succeeds.
    if (error) 
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    const char* state = doc["state"];    
    if (strncmp(state, "OFF", 3)==0) 
    {
      setRelay(0, 1);
    }
    else if (strncmp(state, "ON", 2)==0) 
    {
      setRelay(1, 1);
    }
    else if (strncmp(state, "TOGGLE", 6)==0)
    {
      setRelay(!_relayState, 1);
    }
  }
  else
  {
    _lockDuration = useDefaultIfEmpty(doc["lockDuration"], 20000);
    _cycles = useDefaultIfEmpty(doc["cycles"], 6);
    _delayOn = useDefaultIfEmpty(doc["delayOn"], 500);
    _delayOff = useDefaultIfEmpty(doc["delayOff"], 500);

    setResetMode(1);
  }
}

int useDefaultIfEmpty(int value, int defaultValue)
{
  if(value <= 0)
  {
    return defaultValue;
  }

  return value;
}

#endif