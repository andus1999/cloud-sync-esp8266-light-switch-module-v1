#include "Switch.h"

Switch::Switch(int pinNumber, std::function<void(void)> onFn = nullptr, std::function<void(void)> offFn = nullptr) : pin{pinNumber}
{
  pinMode(pin, INPUT_PULLUP);
  onFunction = onFn;
  offFunction = offFn;
  on = !digitalRead(pin);

  attachInterrupt(
      digitalPinToInterrupt(pin), [this]()
      { this->toggleState(); },
      CHANGE);

  CloudSync::getInstance().watch("switch_" + std::to_string(pin),
                                 std::bind(&Switch::getState, this));
}

int Switch::getState()
{
  return on;
}

void Switch::toggleState()
{
  int pinState = digitalRead(pin);
  if (pinState == on)
  {
    on = !pinState;
    if (on)
      onFunction();
    else
      offFunction();
  }
}
