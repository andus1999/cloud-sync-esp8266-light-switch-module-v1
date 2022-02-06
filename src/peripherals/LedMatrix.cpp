#include "peripherals/LedMatrix.h"

LedMatrix::LedMatrix(int pinDin, int pinCs, int pinClk)
{
  max7219 = Max7219(pinDin, pinCs, pinClk);
  max7219.begin();
  max7219.setBrightness(1);
  CloudSync::getInstance().on("matrix",
                              std::bind(&LedMatrix::display, this, std::placeholders::_1));
  CloudSync::getInstance().on("matrix_brightness",
                              std::bind(&LedMatrix::setBrightnessString, this, std::placeholders::_1));
}

void LedMatrix::setBrightness(float brightness)
{
  max7219.setBrightness(brightness);
}

void LedMatrix::setBrightnessString(std::string brightness)
{
  setBrightness(std::stof(brightness));
}

void LedMatrix::display(std::string value)
{
  unsigned long long bin = std::stoull(value, nullptr, 2);
  unsigned char data[8];
  for (int i = 1; i < 9; i++)
  {
    data[i - 1] = (int)((bin >> (64 - i * 8)) & 0xFF);
  }
  max7219.display(data);
}