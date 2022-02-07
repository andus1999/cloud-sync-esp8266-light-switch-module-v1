#include "CloudSync.h"

CloudSync::CloudSync()
{
  cloudClient = new CloudClient;
  ntpUDP = new WiFiUDP;
  timeClient = new NTPClient(*ntpUDP, "pool.ntp.org");
}

CloudSync &CloudSync::getInstance()
{
  static CloudSync instance;
  return instance;
}

void CloudSync::begin(ESP8266WiFiMulti &m,
                      BearSSL::WiFiClientSecure &c,
                      std::string hardwareId = "none",
                      std::string firmware = "none")
{
  firmwareLink = firmware;
  WiFi.mode(WIFI_AP_STA);

  if (webServer != nullptr)
    delete webServer;
  if (softAp != nullptr)
    delete softAp;
  webServer = new WebServer(m);
  softAp = new SoftAp(m);

  wifiMulti = &m;
  wifiMulti->addAP(STASSID, STAPASS);

  on("firmware",
     std::bind(&CloudSync::handleFirmwareChange, this, std::placeholders::_1));
  on("observers",
     std::bind(&CloudSync::handleObserverChange, this, std::placeholders::_1));
  c.setBufferSizes(4096, 512);
  otaUpdate.begin(&c);
  cloudClient->begin(&c,
                     std::bind(&CloudSync::handleEvent, this, std::placeholders::_1, std::placeholders::_2),
                     hardwareId);
  initialized = true;
  timeClient->begin();
  timeClient->setTimeOffset(0);
  watchLazy("heartbeat", [this]
            {
              this->timeClient->update();
              return this->timeClient->getEpochTime(); });
  webServer->begin();
}

void CloudSync::run()
{
  if (connected && (webServer == nullptr || !webServer->pendingSetup))
  {
    delete webServer;
    webServer = nullptr;
    sync();
    softAp->enableHiddenAp();
  }

  else
  {
    if (webServer == nullptr)
      webServer = new WebServer(*wifiMulti);

    // Enable configuration AP 30 seconds after no connection
    if (millis() - disconnectedSince > 30000)
    {
      webServer->handleClient();
      softAp->enableConfigurationAp();
    }

    // Test every minute if a connection can be reestablished
    if (millis() - lastSync > 60000 && !webServer->pendingSetup)
    {
      Serial.println("Retrying");
      webServer->connected = sync();
    }

    if (webServer->connectionChanged)
    {
      Serial.println("Connection changed.");
      webServer->connected = sync();
      if (webServer->pendingSetup)
      {
        stopSync();
      }
    }
  }
}

bool CloudSync::sync()
{
  if (webServer != nullptr)
    webServer->connectionChanged = false;

  lastSync = millis();

  if (!initialized)
    return false;

  if (wifiMulti->run(10000) == WL_CONNECTED)
  {
    bool updateSuccess = cloudClient->update();
    bool uploadSuccess = true;
    if (millis() - lastUpload > 300000 || (millis() - lastUpload > 15000 && observers))
    {
      uploadSuccess = upload();
    }

    for (auto const &it : watchMap)
    {
      if (syncedLocalState.find(it.first) == syncedLocalState.end())
      {
        uploadSuccess = upload();
        break;
      }
      if (it.second() != syncedLocalState[it.first])
      {
        uploadSuccess = upload();
        break;
      }
    }
    if (updateSuccess && uploadSuccess)
    {
      if (!connected)
      {
        initialized = true;
        Serial.println("Sync: Connection established");
        connected = true;
        connectedSince = millis();
      }
    }
    else if (connected)
    {
      Serial.println("Sync: Connection lost");
      connected = false;
      disconnectedSince = millis();
    }
    return connected;
  }
  Serial.println("Sync: Failed to connect to WiFi.");
  WiFi.disconnect();
  return false;
}

void CloudSync::stopSync()
{
  cloudClient->stop();
}

void CloudSync::on(std::string identifier, EventCallback callback)
{
  eventMap[identifier] = callback;
}

void CloudSync::removeCallback(std::string identifier)
{
  eventMap.erase(identifier);
}

void CloudSync::watch(std::string identifier, std::function<int()> function)
{
  watchMap[identifier] = function;
}

void CloudSync::watchLazy(std::string identifier, std::function<int()> function)
{
  watchLazyMap[identifier] = function;
}

void CloudSync::unWatch(std::string identifier)
{
  watchMap.erase(identifier);
  syncedLocalState.erase(identifier);
}

void CloudSync::override(std::string identifier, int value)
{
  overrides.push_back(std::pair(identifier, value));
}

bool CloudSync::upload()
{
  bool uploadSuccess;
  generateJson();
  if (json != "")
  {
    uploadSuccess = cloudClient->uploadLocalState(json);
    if (uploadSuccess)
    {
      for (auto const &it : valuesInJson)
      {
        if (watchMap.find(it.first) != watchMap.end())
        {
          syncedLocalState[it.first] = valuesInJson[it.first];
        }
        if (watchLazyMap.find(it.first) != watchLazyMap.end())
        {
          syncedLocalState[it.first] = valuesInJson[it.first];
        }
      }
      lastUpload = millis();
    }
  }
  else
  {
    uploadSuccess = true;
    lastUpload = millis();
  }
  return (syncOverrides() && uploadSuccess);
}

bool CloudSync::syncOverrides()
{
  if (overrides.size() == 0)
    return true;
  std::string j = "{";
  for (auto o : overrides)
  {
    j.push_back('\"');
    j.append(o.first);
    j.append("\":");
    j.append(std::to_string(o.second));
    j.push_back(',');
  }
  j.pop_back();
  j.push_back('}');
  Serial.println(j.c_str());
  if (cloudClient->override(j))
  {
    overrides.clear();
    return true;
  }
  return false;
}

void CloudSync::handleEvent(std::string loc, std::string value)
{
  if (eventMap.find(loc) != eventMap.end())
    eventMap[loc](value);
}

void CloudSync::handleFirmwareChange(std::string value)
{
  if (value != firmwareLink && firmwareLink != "none")
  {
    cloudClient->stop();
    delete cloudClient;
    delete timeClient;
    delete ntpUDP;
    delete softAp;
    Serial.println(ESP.getFreeHeap());
    otaUpdate.initiateFirmwareUpdate(value);
  }
}

void CloudSync::handleObserverChange(std::string value)
{
  observers = std::stoi(value);
  if (observers > 0)
    upload();
}

void CloudSync::generateJson()
{
  valuesInJson.clear();
  json = "{";
  for (auto const &it : watchMap)
  {
    addField(it);
  }
  for (auto const &it : watchLazyMap)
  {
    addField(it);
  }
  json.pop_back();
  json.push_back('}');
  if (valuesInJson.size() == 0)
  {
    json = "";
  }
  Serial.println(json.c_str());
}

void CloudSync::addField(std::pair<std::string, std::function<int()>> it)
{
  int value = it.second();
  if (syncedLocalState.find(it.first) != syncedLocalState.end())
  {
    if (syncedLocalState[it.first] == value)
      return;
  }
  std::string field = "\"";
  field.append(it.first);
  field.append("\":");
  field.append(std::to_string(value));
  field.push_back(',');
  if ((json.length() + field.length()) < UPLOAD_BUFFER)
  {
    json.append(field);
    valuesInJson[it.first] = value;
  }
}
