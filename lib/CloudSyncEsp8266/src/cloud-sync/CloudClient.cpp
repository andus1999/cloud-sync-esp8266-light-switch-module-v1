#include "CloudClient.h"

CloudClient::CloudClient()
{
}

CloudClient::~CloudClient()
{
}

void CloudClient::begin(BearSSL::WiFiClientSecure *c,
                        Parser::ParserCallback cb,
                        std::string id)
{
  hardwareId = id;
  client = c;
  parser = Parser(cb, true);

  client->setInsecure();
  client->setNoDelay(true);

  uploadClient.setInsecure();
  uploadClient.setNoDelay(true);
  uploadClient.setBufferSizes(UPLOAD_BUFFER, UPLOAD_BUFFER);

  https.setReuse(true);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  uploadHttps.setReuse(true);
}
bool CloudClient::update()
{
  if (!initialized)
    initialize();
  else
    initialized = true;
  if (millis() - lastRefresh > 3000000)
    refreshIdToken();
  if (initialized)
    return listenForEvents();

  return false;
}

bool CloudClient::uploadLocalState(std::string json)
{
  if (!initialized)
    return false;
  return patch(LOCAL_URI, json);
}

bool CloudClient::override(std::string json)
{
  if (!initialized)
    return false;
  return patch(CLOUD_URI, json);
}

void CloudClient::stop()
{
  listeningForEvents = false;
  initialized = false;
  uploadHttps.end();
  https.end();
  uploadClient.stop();
  client->stop();
}

bool CloudClient::initialize()
{
  initialized = FileSystem::getInstance().getInit() == "true" ? true : false;
  networkUid = FileSystem::getInstance().getNetworkUid();

  if (initialized)
    return true;

  if (!refreshIdToken())
    return false;

  if (!patch(INFO_URI, INITIALIZATION_DATA))
  {
    Serial.println("[HTTPS] PATCH failed couldn't write info.");
    return false;
  }
  Serial.println("Successfully initialized");
  FileSystem::getInstance().setInit("true");
  initialized = true;
  return true;
}

bool CloudClient::patch(std::string uri, std::string body)
{

  if (checkUploadConnection(authenticateUrl(DATABASE + uri)))
    uploadHttps.setURL(authenticateUrl(uri).c_str());

  int httpCode = uploadHttps.PATCH(body.c_str());

  if (httpCode == -2)
  {
    Serial.println("Http code -2. Reconnecting.");
    uploadHttps.end();
    uploadClient.stop();
    uploadHttps.begin(uploadClient, authenticateUrl(DATABASE + uri).c_str());
    httpCode = uploadHttps.PATCH(body.c_str());
  }

  Serial.printf("[HTTPS] PATCH... code: %d\n", httpCode);

  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
  {
    return true;
  }
  else
  {
    Serial.printf("[HTTPS] PATCH... failed, error: %d\n", httpCode);
  }

  return false;
}

bool CloudClient::checkUploadConnection(std::string url)
{
  if (!uploadHttps.connected())
  {
    Serial.println("Upload connection lost, trying to reconnect.");
    uploadHttps.end();
    uploadHttps.begin(uploadClient, url.c_str());
    return false;
  }
  return true;
}

bool CloudClient::checkMainConnection(std::string url)
{
  if (!https.connected())
  {
    Serial.println("Main connection lost, trying to reconnect.");
    https.end();
    https.begin(*client, url.c_str());
    return false;
  }
  return true;
}

bool CloudClient::listenForEvents()
{
  int httpCode = 200;

  if (millis() - lastEvent > 40000 || !listeningForEvents)
  {
    parser.resetParser();
    if (checkMainConnection(authenticateUrl(DATABASE + CLOUD_URI)))
    {
      if (listeningForEvents)
        https.setURL(authenticateUrl(CLOUD_URI).c_str());
      else
        https.setURL(authenticateUrl(DATABASE + CLOUD_URI).c_str());
    }

    https.addHeader("Accept", "text/event-stream");
    httpCode = https.GET();

    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
    {
      Serial.println("Starting to listen for events.");
      listeningForEvents = true;
    }
    else
    {
      listeningForEvents = false;
      Serial.printf("[HTTPS] GET... failed, error: %d\n", httpCode);
    }
  }

  if (listeningForEvents)
  {
    if (https.connected())
    {
      size_t size = client->available();
      if (size)
      {
        lastEvent = millis();
        int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        parser.parse((char *)buff, c);
      }
    }
    return true;
  }
  return false;
}

bool CloudClient::refreshIdToken()
{
  Serial.println("Refreshing id token");
  listeningForEvents = false;

  if (checkMainConnection(REFRESH_URL))
    https.setURL(REFRESH_URL);

  std::string body = "{\"grant_type\":\"refresh_token\",\"refresh_token\":\"" + FileSystem::getInstance().getRefreshToken() + "\"}";

  int httpCode = https.POST(body.c_str());
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
  {
    Parser p([this](std::string loc, std::string value)
             {
               if (loc == "id_token")
                 this->idToken = value; });

    while (https.connected())
    {
      String chunkHeader = client->readStringUntil('\n');
      chunkHeader.trim();
      int chunkSize = std::stoi(chunkHeader.c_str(), nullptr, 16) + 2;
      if (chunkSize == 2)
      {
        lastRefresh = millis();
        Serial.println("Successfully refreshed id token.");
        return true;
      }
      while (chunkSize)
      {
        size_t size = client->available();
        int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        p.parse((char *)buff, c);
        chunkSize -= c;
      }
    }
  }
  return false;
}

std::string CloudClient::authenticateUrl(std::string url)
{
  // return url;
  return url + "?auth=" + idToken;
}
