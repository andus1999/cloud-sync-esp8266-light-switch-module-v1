#pragma once
#include <string>
#include <ESP8266httpUpdate.h>
#include "../utils/FileSystem.h"

class OtaUpdate
{
public:
  OtaUpdate();
  void initiateFirmwareUpdate(std::string firmwareLink);
  void begin(BearSSL::WiFiClientSecure *c);

private:
  BearSSL::WiFiClientSecure *client;
};