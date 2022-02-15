#include "Button.h"

Button::Button(int pinNumber, std::function<void(void)> a = nullptr) : pin{pinNumber}
{
  pinMode(pin, INPUT);

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
    lastActivation = millis();
    performAction += 1;
    timesActivated += 1;
  }
}
