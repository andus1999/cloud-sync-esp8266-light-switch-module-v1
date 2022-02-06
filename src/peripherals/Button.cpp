#include "peripherals/Button.h"

Button::Button(int pinNumber, std::function<void(void)> a = nullptr) : pin{pinNumber}
{
  action = a;
  attachInterrupt(
      digitalPinToInterrupt(pin), [this]()
      { this->increaseActivationCount(); },
      RISING);

  CloudSync::getInstance().watch("button_" + std::to_string(pin),
                                 std::bind(&Button::getActivationCount, this));
}

int Button::getActivationCount()
{
  if (performAction)
  {
    performAction -= 1;
    action();
  }
  return timesActivated;
}

void Button::increaseActivationCount()
{
  if (millis() - lastActivation > 100)
  {
    performAction += 1;
    lastActivation = millis();
    timesActivated += 1;
  }
}
