#pragma once
#include <map>
#include <list>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Firmware.h"
#include "cloud_sync/CloudClient.h"
#include "cloud_sync/OtaUpdate.h"

class CloudSync
{
public:
  typedef std::function<void(std::string)> EventCallback;
  static CloudSync &getInstance();
  void handleEvent(std::string loc, std::string value);
  void on(std::string identifier, EventCallback);
  void removeCallback(std::string identifier);
  void watch(std::string identifier, std::function<int()>);
  void watchLazy(std::string identifier, std::function<int()>);
  void unWatch(std::string identifier);
  void override(std::string identifier, int value);
  bool sync(void);
  void begin(ESP8266WiFiMulti &m,
             BearSSL::WiFiClientSecure &c);
  bool initialize();
  void stop();
  bool started = false;
  bool connectionChanged = false;
  bool initialized = false;
  bool connected = false;
  unsigned long lastSync = -60000;
  unsigned long connectedSince = -60000;
  unsigned long disconnectedSince = 0;

  CloudSync(CloudSync const &) = delete;
  void operator=(CloudSync const &) = delete;

private:
  CloudSync();
  ~CloudSync();
  ESP8266WiFiMulti *wifiMulti;
  CloudClient *cloudClient;
  OtaUpdate otaUpdate;
  WiFiUDP *ntpUDP;
  NTPClient *timeClient;
  std::map<std::string, CloudSync::EventCallback> eventMap;
  std::map<std::string, std::function<int()>> watchMap;
  std::map<std::string, std::function<int()>> watchLazyMap;
  std::map<std::string, int> syncedLocalState;
  std::list<std::pair<std::string, int>> overrides;
  void generateJson();
  void addField(std::pair<std::string, std::function<int()>> it);
  std::string json;
  std::map<std::string, int> valuesInJson;
  void handleFirmwareChange(std::string value);
  void handleObserverChange(std::string value);
  bool syncOverrides();
  int observers = 0;
  unsigned long lastUpload = -300000;
  bool upload();
};
