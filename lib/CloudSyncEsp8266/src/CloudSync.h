#pragma once
#include <map>
#include <list>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
// #include <WiFiUdp.h>

#include "cloud-sync/CloudClient.h"
#include "cloud-sync/OtaUpdate.h"
// #include "cloud-sync/NTPClient.h"
#include "cloud-sync/WebServer.h"
#include "cloud-sync/SoftAp.h"

class CloudSync
{
public:
  typedef std::function<void(std::string)> EventCallback;
  static CloudSync &getInstance();

  void run();
  void handleEvent(std::string loc, std::string value);
  void on(std::string identifier, EventCallback);
  void removeCallback(std::string identifier);
  void watch(std::string identifier, std::function<int()>);
  void watchLazy(std::string identifier, std::function<int()>);
  void unWatch(std::string identifier);
  void override(std::string identifier, int value);
  bool sync(void);
  void begin(ESP8266WiFiMulti &m,
             BearSSL::WiFiClientSecure &c,
             std::string hardwareId,
             std::string firmwareLink);

  bool initialized = false;
  bool connected = false;

  CloudSync(CloudSync const &) = delete;
  void operator=(CloudSync const &) = delete;

private:
  CloudSync();
  enum SyncState
  {
    STARTING,
    CONNECTED,
    DISCONNECTED,
    PENDING_SETUP,
    SETUP_SUCCESS,
    SETUP_FAILURE,
  };
  SyncState parserState;

  ESP8266WiFiMulti *wifiMulti;

  OtaUpdate otaUpdate;
  CloudClient *cloudClient;

  // WiFiUDP *ntpUDP;
  // NTPClient *timeClient;

  WebServer *webServer = nullptr;
  SoftAp *softAp = nullptr;

  void generateJson();
  void addField(std::pair<std::string, std::function<int()>> it);
  void handleFirmwareChange(std::string value);
  void handleUpdate(std::string value);
  void handleCommand(std::string command);
  bool syncOverrides();
  void stopSync();
  bool upload();

  std::map<std::string, CloudSync::EventCallback> eventMap;
  std::map<std::string, std::function<int()>> watchMap;
  std::map<std::string, std::function<int()>> watchLazyMap;
  std::map<std::string, int> syncedLocalState;
  std::map<std::string, int> overrides;
  std::map<std::string, bool> ignoreEvents;
  std::map<std::string, int> valuesInJson;
  std::string json;
  std::string firmwareLink;

  unsigned int timeStamp;
  bool updateRequested = false;
  unsigned long lastUpload = -300000;
  unsigned long lastSync = -60000;
  unsigned long connectedSince = -60000;
  unsigned long disconnectedSince = -60000;
};
