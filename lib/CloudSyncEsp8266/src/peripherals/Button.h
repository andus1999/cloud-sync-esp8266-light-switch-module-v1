#pragma once

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <functional>
#include "../CloudSync.h"

class Button
{
  // Standard push button
public:
  Button(int pinNumber);
  Button(int pinNumber, std::function<void(void)>);
  int pin;
  int timesActivated = 0;

private:
  std::function<void(void)> action;
  int performAction = 0;
  long lastActivation = -1000;
  int getActivationCount();
  void increaseActivationCount();
};