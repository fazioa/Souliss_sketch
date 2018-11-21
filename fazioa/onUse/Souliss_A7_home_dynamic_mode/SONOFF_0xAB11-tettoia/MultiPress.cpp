#include "MultiPress.h"

SimplePress* SimplePress::instances[MAX_BUTTON_INSTANCES];
uint8_t SimplePress::instanceCount = 0;

SimplePress::SimplePress(int _pin, uint32_t _pressInterval, void(*_callBack)(const int value))
{
  callBackWithArg = _callBack;
  pin = _pin;
  debouncePeriod = DEFAULT_DEBOUNCE_MILLISECONDS;
  pressInterval = _pressInterval;
  instances[instanceCount] = this;
  instanceCount++;
}

SimplePress::SimplePress(int _pin, uint32_t _pressInterval, void(*_callBack)(void))
{
  callBack = _callBack;
  pin = _pin;
  debouncePeriod = DEFAULT_DEBOUNCE_MILLISECONDS;
  pressInterval = _pressInterval;
  instances[instanceCount] = this;
  instanceCount++;
}

bool SimplePress::begin()
{
  pinMode(pin, INPUT_PULLUP);
  lastState = HIGH;
  Serial.println(pin);
  return true;
}

bool SimplePress::beginAll()
{
  for (size_t button = 0; button < instanceCount; button++)
  {
    instances[button]->begin();
  }
  return true;
}

void SimplePress::update(void)
{
  for (size_t button = 0; button < instanceCount; button++)
  {
    int8_t pressed = instances[button]->pressed();
    if(pressed)
    {
      if(instances[button]->callBackWithArg)
      {
        instances[button]->callBackWithArg(pressed);
      }
      else if(instances[button]->callBack)
      {
        instances[button]->callBack();
      }
    }
  }
}

int8_t SimplePress::pressed()
{
  byte nowState = digitalRead(pin);
  if(nowState != lastState)
  {
    //Serial.println(F("CHANGED STATE"));
    if(millis() - lastMillis < debouncePeriod) return 0;
    if(nowState == LOW)
    {
      lastMillis = millis();
      pressCount++;
    }
    else
    {
      if (millis() - lastMillis > pressInterval) // a long press
      {
        lastState = nowState;
        pressCount = 0;
        return -1;
      }
    }
  }
  if(pressCount != 0)
  {
    if(millis() - lastMillis > pressInterval and nowState == HIGH)
    {
      int presses = pressCount;
      pressCount = 0;
      return presses;
    }
  }
  lastState = nowState;
  return 0;
}

int SimplePress::setDebounce(uint8_t dbounce)
{
  debouncePeriod = dbounce;
  return debouncePeriod;
}

void SimplePress::setDebounceAll(uint8_t dbounce)
{
  for (size_t button = 0; button < instanceCount; button++)
  {
    instances[button]->setDebounce(dbounce);
  }
}

int SimplePress::getCount()
{
  return instanceCount;
}
