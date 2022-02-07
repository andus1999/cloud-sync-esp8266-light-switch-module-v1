#include "OtaUpdate.h"

OtaUpdate::OtaUpdate()
{
}

void OtaUpdate::begin(BearSSL::WiFiClientSecure *c)
{
  client = c;
}

void OtaUpdate::initiateFirmwareUpdate(std::string firmwareLink)
{
  FileSystem::getInstance().setInit("false");

  client->setBufferSizes(16384, 512);
  Serial.println("Updating firmware...");
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  t_httpUpdate_return ret = ESPhttpUpdate.update(*client, firmwareLink.c_str());

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}
