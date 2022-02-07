#pragma once

#include <string>
#include <map>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "../CloudSync.h"

class WiFiButton
{
private:
  static void onDataRecieve(uint8_t *mac, uint8_t *incomingData, uint8_t len);
  static std::map<std::string, int> timesActivatedMap;
  static std::map<std::string, int> performActionMap;
  std::function<void(void)> action;
  int getActivationCount();
  void increaseActivationCount();
  static std::string macToString(uint8_t *mac);

public:
  WiFiButton(std::array<uint8_t, 6>);
  WiFiButton(std::array<uint8_t, 6>, std::function<void(void)>);
  std::string mac;
  void begin();
};