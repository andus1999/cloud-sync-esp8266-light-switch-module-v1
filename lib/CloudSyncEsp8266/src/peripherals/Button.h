#pragma once

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <functional>
#include "../CloudSync.h"

class Button
{
  // Standard push button
public:
  Button(int pinNumber, std::function<void(void)>);
  int getActivationCount();

private:
  int pin;
  int timesActivated = 0;
  std::function<void(void)> action;
  unsigned long lastActivation = -1000;
  void increaseActivationCount();
};