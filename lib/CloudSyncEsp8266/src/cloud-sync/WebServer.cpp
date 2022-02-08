#include "WebServer.h"

WebServer::WebServer(ESP8266WiFiMulti &w)
{
  wifiMulti = &w;
  fileSystem = &FileSystem::getInstance();
  ssid = fileSystem->getSsid();
  pass = fileSystem->getPass();
  networkUid = fileSystem->getNetworkUid();
  refreshToken = fileSystem->getRefreshToken();
}

void WebServer::begin()
{
  Serial.println("Starting server.");

  // server.getServer().setRSACert(new BearSSL::X509List(fileSystem->getServerCert().c_str()), new BearSSL::PrivateKey(fileSystem->getServerKey().c_str()));
  // server.getServer().setCache(&serverCache);
  // server.getServer().setBufferSizes(1024, 1024);

  // server.on("/ping", [this]
  //           { this->handleConnectionTest(); });
  // server.on("/credentials", [this]
  //           { this->handleSetCredentials(); });
  // server.on("/network-information", [this]
  //           { this->handleSetNetworkInformation(); });
  server.on("/success", std::bind(&WebServer::handleSuccess, this));
  server.on("/initialize", [this]
            { this->handleInitialize(); });
  server.on("/", [this]
            { this->handleIndex(); });
  server.on("/font.tff", [this]
            { this->handleFont(); });
  server.begin(80);
  initialized = true;
  if (!pendingSetup)
  {
    connectToAp();
  }
}

void WebServer::handleClient()
{
  if (!initialized)
    begin();
  if (server.getServer().status() == CLOSED)
  {
    Serial.println("restarting the server");
    server.begin(80);
  }
  Serial.print("handle");
  server.handleClient();
}

void WebServer::handleResponse()
{
  Serial.println("New request.");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
}

void WebServer::handleIndex()
{
  pendingSetup = true;
  handleResponse();
  server.send(200, "text/html", fileSystem->getHtml().c_str());
}

void WebServer::handleFont()
{
  handleResponse();
  server.send(200, "text/html", fileSystem->getFont().c_str());
}

void WebServer::handleInitialize()
{
  if (server.method() == HTTP_PUT)
  {
    fileSystem->setInit("false");
    std::string data = server.arg("plain").c_str();

    // networkUid
    int delimiterPosition = data.find(',');
    networkUid = data.substr(0, delimiterPosition);
    data = data.substr(delimiterPosition + 1, data.length());

    // refreshToken
    delimiterPosition = data.find(",");
    refreshToken = data.substr(0, delimiterPosition);
    data = data.substr(delimiterPosition + 1, data.length());

    // ssid and pass
    delimiterPosition = data.find(",");
    ssid = data.substr(0, delimiterPosition);
    pass = data.substr(delimiterPosition + 1, data.length());

    fileSystem->setNetworkUid(networkUid);
    fileSystem->setRefreshToken(refreshToken);
    fileSystem->setSsid(ssid);
    fileSystem->setPass(pass);

    handleResponse();
    Serial.println("Initialization recieved.");
    server.send(200, "text/plain", "ok");
    connectToAp();
  }
  else
  {
    handleResponse();
    server.send(200, "text/plain", "ok");
  }
}

void WebServer::handleSuccess()
{
  handleResponse();
  bool c = connected;
  server.send(200, "text/plain", c ? "true" : "false");
  pendingSetup = false;
  if (c)
  {
    connectionChanged = true;
  }
}

// void WebServer::handleConnectionTest()
// {
//   handleResponse();
//   server.send(200, "text/plain", WiFi.macAddress());
//   pendingSetup = true;
// }

// void WebServer::handleSetCredentials()
// {
//   if (server.method() == HTTP_PUT)
//   {
//     std::string cred = server.arg("plain").c_str();
//     int delimiterPosition = cred.find(',');
//     ssid = cred.substr(0, delimiterPosition);
//     pass = cred.substr(delimiterPosition + 1, cred.length());
//     fileSystem->setSsid(ssid);
//     fileSystem->setPass(pass);
//     handleResponse();
//     Serial.println("WiFi credentials recieved.");
//     server.send(200, "text/plain", "true");
//     connectToAp();
//   }
//   else
//   {
//     handleResponse();
//     server.send(200, "text/plain", "wrong method");
//   }
// }

// void WebServer::handleSetNetworkInformation()
// {
//   if (server.method() == HTTP_PUT)
//   {
//     std::string info = server.arg("plain").c_str();
//     int delimiterPosition = info.find(',');
//     networkUid = info.substr(0, delimiterPosition);
//     refreshToken = info.substr(delimiterPosition + 1, info.length());
//     fileSystem->setNetworkUid(networkUid);
//     fileSystem->setRefreshToken(refreshToken);
//     Serial.println("Iot network information recieved.");
//     bool connected = WiFi.status() == WL_CONNECTED;

//     handleResponse();
//     server.send(200, "text/plain", connected ? "wifi_connected" : "wifi_disconnected");
//     FileSystem::getInstance().setInit("false");
//     if (connected)
//     {
//       connectToAp();
//     }
//   }
//   else
//   {
//     handleResponse();
//     server.send(200, "text/plain", "wrong method");
//   }
// }

void WebServer::connectToAp()
{
  Serial.println(ssid.c_str());
  Serial.println(pass.c_str());

  if (ssid != "" && pass != "")
  {
    Serial.println("Adding access point...");
    wifiMulti->addAP(ssid.c_str(), pass.c_str());
  }

  if (networkUid != "" && refreshToken != "")
  {
    Serial.println("Starting sync...");
    server.close();
    // cuases OOM
    connectionChanged = true;
  }
}