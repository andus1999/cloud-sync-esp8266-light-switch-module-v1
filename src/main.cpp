#include <string>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

#include "utils/FileSystem.h"
#include "node/WebServer.h"
#include "node/SoftAp.h"
#include "cloud_sync/CloudSync.h"
#include "peripherals/Led.h"
#include "peripherals/Button.h"
#include "peripherals/WiFiButton.h"

#include <espnow.h>

ESP8266WiFiMulti wifiMulti;
BearSSL::WiFiClientSecure client;
WebServer server(wifiMulti);
SoftAp softAp(wifiMulti);
CloudSync *cloudSync = &CloudSync::getInstance();

// Led led(0);
// Button button(2, []
//               { led.toggle(); });
// WiFiButton wifiButton({0x84, 0xf3, 0xeb, 0xc9, 0x27, 0x4a}, []
//                       { led.toggle(); });

void setup()
{
  Serial.begin(115200);

  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

  WiFi.mode(WIFI_AP_STA);
  wifiMulti.addAP(STASSID, STAPASS);
  server.begin();
  cloudSync->begin(wifiMulti, client);
}

void loop()
{
  if (cloudSync->connected && !server.pendingSetup)
  {
    cloudSync->sync();
    softAp.enableHiddenAp();
  }

  else
  {

    // Enable configuration AP 30 seconds after no connection
    if (millis() - cloudSync->disconnectedSince > 30000)
    {
      server.handleClient();
      softAp.enableConfigurationAp();
    }

    // Test every minute if a connection can be reestablished
    if (millis() - cloudSync->lastSync > 60000 && !server.pendingSetup)
    {
      Serial.println("Retrying");
      cloudSync->sync();
    }

    if (cloudSync->connectionChanged)
    {
      Serial.println("Connection changed.");
      cloudSync->sync();
      if (server.pendingSetup)
      {
        cloudSync->stop();
      }
    }
  }
  Serial.print("Free Memory: " + String(ESP.getFreeHeap()) + " B\r");
  delay(10);
}