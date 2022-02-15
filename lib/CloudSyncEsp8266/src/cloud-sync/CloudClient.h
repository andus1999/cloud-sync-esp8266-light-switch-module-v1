#pragma once
#include <string>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include "../utils/Parser.h"
#include "../utils/FileSystem.h"

#ifndef HARDWARE_ID
#define HARDWARE_ID "none"
#endif

#define REFRESH_URL "https://securetoken.googleapis.com/v1/token?key=AIzaSyC05AlpCk7PBx8XjUElx5QXhPFsT9adWy4"
#define DATABASE "https://cloud-sync-iot-default-rtdb.europe-west1.firebasedatabase.app"
#define NETWORK_PATH "/networks/" + networkUid + "/devices/" + std::string(WiFi.macAddress().c_str())
#define INFO_URI std::string(NETWORK_PATH + "/info.json")
#define LOCAL_URI std::string(NETWORK_PATH + "/local_state.json")
#define CLOUD_URI std::string(NETWORK_PATH + "/cloud_state.json")
#define INITIALIZATION_DATA "{\"hardware_id\":\"" + hardwareId + "\"}"
#define UPLOAD_BUFFER 512

class CloudClient
{
public:
  CloudClient();
  ~CloudClient();
  void begin(BearSSL::WiFiClientSecure *client,
             Parser::ParserCallback,
             std::string hardwareId);
  bool initialize();
  bool update(void);
  bool uploadLocalState(std::string);
  bool override(std::string);
  void stop(void);

private:
  ESP8266WiFiMulti *wifiMulti;
  BearSSL::WiFiClientSecure *client;
  Parser parser;
  uint8_t buff[128] = {0};
  HTTPClient https;
  HTTPClient uploadHttps;
  BearSSL::WiFiClientSecure uploadClient;
  unsigned long lastEvent = 0;
  bool initialized = false;
  bool listeningForEvents = false;
  std::string hardwareId;
  std::string networkUid;

  bool patch(std::string uri, std::string body);
  bool checkUploadConnection(std::string url);

  bool listenForEvents();
  bool checkMainConnection(std::string url);

  std::string authenticateUrl(std::string url);

  bool refreshIdToken();
  std::string idToken;
  unsigned long lastRefresh = -3000000;
};