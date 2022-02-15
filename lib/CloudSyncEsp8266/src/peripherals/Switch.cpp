#include "Switch.h"

Switch::Switch(int pinNumber, std::function<void(void)> onFn = nullptr, std::function<void(void)> offFn = nullptr) : pin{pinNumber}
{
  pinMode(pin, INPUT);
  onFunction = onFn;
  offFunction = offFn;

  attachInterrupt(
      digitalPinToInterrupt(pin), [this]()
      { this->toggleState(); },
      CHANGE);

  CloudSync::getInstance().watch("switch_" + std::to_string(pin),
                                 std::bind(&Switch::getState, this));
}

int Switch::getState()
{
  if (onState != on)
  {
    on = !on;
    if (on)
      onFunction();
    else
      offFunction();
  }
  return on;
}

void Switch::toggleState()
{
  int pinState = digitalRead(pin);
  if (pinState == onState)
  {
    lastActivation = millis();
    onState = !pinState;
  }
}
