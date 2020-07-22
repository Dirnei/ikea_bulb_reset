
#define RELAY 12
#define GREEN_LED 13
#define BUTTON 0

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
      setRelay(!_relayState);
    }
    
  }
}

void setResetMode(int state)
{
  _resetMode = state;
  digitalWrite(GREEN_LED, !state);

  if(state == 1)
  {
    setRelay(1);
  }
}

void setRelay(int state)
{
  if(_isLocked)
  {
    return;
  }

  _relayState = state;
  digitalWrite(RELAY, state);
}

void loop()
{
    if(_resetMode == 1)
    {
      performReset();
      lockRelayFor(20000);
    }

    if(_isLocked && _lockedUntil < millis())
    {
      _isLocked = 0;
    }
}

void performReset()
{
  for(int i = 0; i < 6; i++)
  {
    setRelay(0);
    delay(500);
    setRelay(1);
    delay(500);
  }
}

void lockRelayFor(int duration)
{
  _isLocked = 1;
  _lockedUntil = millis() + duration;
}