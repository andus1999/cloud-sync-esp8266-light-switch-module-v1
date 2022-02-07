#pragma once

#include <ESP8266WebServerSecure.h>
#include <ESP8266WiFiMulti.h>
#include <DNSServer.h>
#include "../utils/FileSystem.h"

class WebServer
{
private:
  bool initialized = false;
  bool running = false;
  void handleInitialize();
  void handleIndex();
  void handleFont();
  void handleSetCredentials();
  void handleSetNetworkInformation();
  void handleConnectionTest();
  void handleSuccess();
  void handleResponse();
  void finalizeRequest();
  void connectToAp();
  ESP8266WebServer server;
  BearSSL::ServerSessions serverCache{2};
  ESP8266WiFiMulti *wifiMulti;
  FileSystem *fileSystem;

public:
  bool pendingSetup = false;
  bool connected = false;
  bool connectionChanged = false;
  WebServer(ESP8266WiFiMulti &wifiMulti);
  void begin();
  void handleClient();
  std::string ssid;
  std::string pass;
  std::string networkUid;
  std::string refreshToken;
};