#pragma once
#include <Arduino.h>

#define REG_SCAN_LIMIT 0x0b
#define REG_DECODE 0x09
#define REG_INTENSITY 0x0a
#define REG_SCAN_LIMIT 0x0b
#define REG_SHUTDOWN 0x0c
#define REG_DISPLAY_TEST 0x0f

#define INTENSITY_MIN 0x00
#define INTENSITY_MAX 0x0F

class Max7219 {
public:
  Max7219(int din, int cs, int clk);
  Max7219() = default;
  void display(unsigned char *data);
  void setBrightness(float brightness);
  void begin();
  void clear();

private :
  int din;
  int cs;
  int clk;
  void write(unsigned char opcode, unsigned char data);
};
