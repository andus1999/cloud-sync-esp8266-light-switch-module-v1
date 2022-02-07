#pragma once

#define STASSID "Node"
#define STAPASS "RcG#h[fLdnb2HY2{"
// #define STAPASS ""

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>

class SoftAp
{
public:
  SoftAp(ESP8266WiFiMulti &wifiMulti);
  void enableConfigurationAp();
  void enableHiddenAp();

private:
  typedef enum
  {
    OFF = 0,
    CONFIGURATION = 1,
    HIDDEN = 2,
  } ApState;
  ESP8266WiFiMulti *wifiMulti;
  ApState state = OFF;
};