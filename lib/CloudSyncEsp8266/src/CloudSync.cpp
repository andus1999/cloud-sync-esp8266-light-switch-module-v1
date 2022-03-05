#include "CloudSync.h"

CloudSync::CloudSync()
{
  cloudClient = new CloudClient;
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

  if (softAp != nullptr)
    delete softAp;
  softAp = new SoftAp(m);

  wifiMulti = &m;
  wifiMulti->addAP(STASSID, STAPASS);

  on("firmware",
     std::bind(&CloudSync::handleFirmwareChange, this, std::placeholders::_1));
  on("update",
     std::bind(&CloudSync::handleUpdate, this, std::placeholders::_1));
  on("command", std::bind(&CloudSync::handleCommand, this, std::placeholders::_1));

  c.setBufferSizes(4096, 512);
  otaUpdate.begin(&c);
  cloudClient->begin(&c,
                     std::bind(&CloudSync::handleEvent, this, std::placeholders::_1, std::placeholders::_2),
                     hardwareId);
  initialized = true;
  watchLazy("heartbeat", [this]
            { return this->timeStamp; });
}

void CloudSync::run()
{
  if (connected && (webServer == nullptr || !webServer->pendingSetup))
  {
    if (webServer != nullptr)
    {
      delete webServer;
      webServer = nullptr;
    }
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

    if (webServer->connectionChanged)
    {
      // OOM issue
      Serial.println("Connection changed.");
      webServer->connected = sync();
      if (webServer->pendingSetup || webServer->connected == false)
      {
        stopSync();
      }
    }

    // Test every minute if a connection can be reestablished
    if (millis() - lastSync > 300000 && !webServer->pendingSetup)
    {
      Serial.println("Retrying");
      webServer->connected = sync();
      stopSync();
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
    if (millis() - lastUpload > 300000 || (millis() - lastUpload > 15000 && updateRequested))
    {
      updateRequested = false;
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
      stopSync();
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
  overrides[identifier] = value;
}

void CloudSync::handleEvent(std::string loc, std::string value)
{
  Serial.println("Event");
  Serial.println(loc.c_str());
  Serial.println(value.c_str());

  if (ignoreEvents.find(loc) != ignoreEvents.end() && ignoreEvents[loc])
  {
    Serial.println("Ignoring event");
    ignoreEvents[loc] = false;
    return;
  }

  Serial.println();

  if (eventMap.find(loc) != eventMap.end())
    eventMap[loc](value);
}

void CloudSync::handleCommand(std::string command)
{
  if (command == "disconnect")
  {
    override("command", 0);
    syncOverrides();
    ignoreEvents["command"] = false;
    cloudClient->stop();
    FileSystem::getInstance().setNetworkUid("");
    FileSystem::getInstance().setRefreshToken("");
    FileSystem::getInstance().setInit("false");
    connected = false;
    disconnectedSince = millis() - 300000;
  }
}

void CloudSync::handleFirmwareChange(std::string value)
{
  if (value != firmwareLink && firmwareLink != "none")
  {
    cloudClient->stop();
    delete cloudClient;
    delete softAp;
    Serial.println(ESP.getFreeHeap());
    otaUpdate.initiateFirmwareUpdate(value);
  }
}

void CloudSync::handleUpdate(std::string value)
{
  updateRequested = true;
  timeStamp = std::stoi(value);
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
  for (auto const &o : overrides)
  {
    j.push_back('\"');
    j.append(o.first);
    j.append("\":");
    j.append(std::to_string(o.second));
    j.push_back(',');
  }
  j.pop_back();
  j.push_back('}');
  Serial.println("Overrides");
  Serial.println(j.c_str());
  Serial.println();

  if (cloudClient->override(j))
  {
    for (auto const &o : overrides)
    {
      ignoreEvents[o.first] = true;
    }
    overrides.clear();
    return true;
  }
  return false;
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
  Serial.println("Upload");
  Serial.println(json.c_str());
  Serial.println();
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
