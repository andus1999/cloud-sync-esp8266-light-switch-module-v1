#include "SoftAp.h"

SoftAp::SoftAp(ESP8266WiFiMulti &w)
{
  ip_napt_init(IP_NAPT_MAX / 16, IP_PORTMAP_MAX / 16);
  wifiMulti = &w;
  WiFi.softAPConfig(
      IPAddress(172, 16, 0, 1),
      IPAddress(172, 16, 0, 1),
      IPAddress(255, 255, 0, 0));
}

void SoftAp::enableConfigurationAp()
{
  if (state != CONFIGURATION)
  {
    state = CONFIGURATION;
    Serial.println(WiFi.softAP(STASSID, "") ? "Config AP ready" : "Config AP failed!");
  }
}

void SoftAp::enableHiddenAp()
{
  if (wifiMulti->run() == WL_CONNECTED && state != HIDDEN)
  {
    state = HIDDEN;
    Serial.println(WiFi.softAP(STASSID, STAPASS, 1, 1, 2) ? "Network ready" : "Network failed");
    dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
    dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));

    Serial.printf("Heap before napt init: %d\n", ESP.getFreeHeap());
    err_t ret = ip_napt_enable_no(SOFTAP_IF, 1);
    if (ret == ERR_OK)
    {
      Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID, WiFi.SSID().c_str());
    }
    Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
    if (ret != ERR_OK)
    {
      Serial.printf("NAPT initialization failed\n");
    }
  }
}