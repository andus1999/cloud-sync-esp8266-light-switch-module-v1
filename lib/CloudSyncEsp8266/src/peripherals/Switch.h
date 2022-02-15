#pragma once

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <functional>
#include "../CloudSync.h"

class Switch
{
  // Standard switch
public:
  Switch(int pinNumber, std::function<void(void)> on, std::function<void(void)> off);
  int pin;
  bool on = false;

private:
  std::function<void(void)> onFunction;
  std::function<void(void)> offFunction;
  unsigned long lastActivation = -1000;
  bool onState = false;
  int getState();
  void toggleState();
};