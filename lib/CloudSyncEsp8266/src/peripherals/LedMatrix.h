#pragma once

#include <Arduino.h>
#include <drivers/Max7219.h>
#include "../CloudSync.h"

class LedMatrix
{
public:
  LedMatrix(int pinDin, int pinCs, int pinClk);
  void setBrightness(float brightness);

private:
  Max7219 max7219;
  void setBrightnessString(std::string brightness);
  void display(std::string data);
};