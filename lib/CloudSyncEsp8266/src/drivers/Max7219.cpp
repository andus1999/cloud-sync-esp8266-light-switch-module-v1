#include "../drivers/Max7219.h"

Max7219::Max7219(int pinDin, int pinCs, int pinClk)
{
  din = pinDin;
  cs = pinCs;
  clk = pinClk;
  pinMode(din, OUTPUT);
  pinMode(cs, OUTPUT);
  pinMode(clk, OUTPUT);
}

void Max7219::begin()
{
  digitalWrite(cs, HIGH);
  write(REG_SCAN_LIMIT, 7);
  write(REG_DECODE, 0x00);
  write(REG_SHUTDOWN, 1);
  write(REG_DISPLAY_TEST, 0);
  clear();
}

void Max7219::display(unsigned char *data)
{
  begin();
  for (int i = 0; i < 8; i++)
  {
    write(i + 1, data[i]);
  }
}

void Max7219::write(unsigned char opcode, unsigned char data)
{
  digitalWrite(cs, LOW);
  shiftOut(din, clk, MSBFIRST, opcode);
  shiftOut(din, clk, MSBFIRST, data);
  digitalWrite(cs, HIGH);
}

void Max7219::clear()
{
  for (int i = 0; i < 8; i++)
  {
    write(i + 1, 0x00);
  }
}

void Max7219::setBrightness(float brightness)
{
  char b = brightness * 16;
  if (b > 15)
    b = 15;
  if (b < 0)
    b = 0;
  write(REG_INTENSITY, b);
}
