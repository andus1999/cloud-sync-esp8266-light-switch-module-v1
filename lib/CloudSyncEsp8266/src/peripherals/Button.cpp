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
  return timesActivated;
}

void Button::increaseActivationCount()
{
  if (millis() - lastActivation > 10)
  {
    lastActivation = millis();
    action();
    timesActivated += 1;
  }
}
