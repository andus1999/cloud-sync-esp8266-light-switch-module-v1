#include "Led.h"

Led::Led(int pin) : _pin{pin}
{
  pinMode(pin, OUTPUT);
  std::string id = "pin_" + std::to_string(pin);
  CloudSync::getInstance().on(id, std::bind(&Led::set, this, std::placeholders::_1));
  CloudSync::getInstance().watch(id, [this]
                                 { return digitalRead(this->_pin); });
}

void Led::set(std::string value)
{
  if (value == "1")
    digitalWrite(_pin, HIGH);
  else
    digitalWrite(_pin, LOW);
}

void Led::toggle()
{
  int newValue = !digitalRead(_pin);
  digitalWrite(_pin, newValue);
  CloudSync::getInstance().override("pin_" + std::to_string(_pin), newValue);
}
