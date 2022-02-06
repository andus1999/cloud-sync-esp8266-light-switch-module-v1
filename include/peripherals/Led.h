#pragma once

#include <Arduino.h>
#include <string>
#include "cloud_sync/CloudSync.h"

class Led
{

public:
  Led(int pin);
  void toggle();

private:
  int _pin;
  void set(std::string value);
};