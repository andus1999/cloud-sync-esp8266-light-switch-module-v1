#include "peripherals/WiFiButton.h"

std::map<std::string, int> WiFiButton::timesActivatedMap;
std::map<std::string, int> WiFiButton::performActionMap;

WiFiButton::WiFiButton(std::array<uint8_t, 6> m, std::function<void(void)> a = nullptr)
{
  uint8_t macArray[6];
  memcpy(macArray, m.data(), sizeof(macArray));
  mac = macToString(macArray);
  performActionMap[mac] = 0;
  action = a;

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(WiFiButton::onDataRecieve);

  CloudSync::getInstance().watch("button_" + mac,
                                 std::bind(&WiFiButton::getActivationCount, this));
}

void WiFiButton::begin()
{
}

std::string WiFiButton::macToString(uint8_t *m)
{
  std::string macString = "";
  for (int i = 0; i < 6; i++)
  {
    char buffer[3];
    sprintf(buffer, "%02X", m[i]);
    macString.append(buffer);
    macString.push_back(':');
  }
  macString.pop_back();
  return macString;
}

void WiFiButton::onDataRecieve(uint8_t *m, uint8_t *incomingData, uint8_t len)
{
  std::string mac = macToString(m);
  if (performActionMap.find(mac) == performActionMap.end())
    return;
  uint8_t timesActivated;
  memcpy(&timesActivated, incomingData, sizeof(timesActivated));
  if (timesActivatedMap.find(mac) == timesActivatedMap.end())
  {
    timesActivatedMap[mac] = timesActivated - 1;
  }
  if (timesActivated - timesActivatedMap[mac] != 0)
    performActionMap[mac] += 1;
  timesActivatedMap[mac] = timesActivated;
}

int WiFiButton::getActivationCount()
{
  if (performActionMap[mac])
  {
    Serial.println("WifiButtonAction");
    performActionMap[mac] -= 1;
    action();
  }
  if (timesActivatedMap.find(mac) != timesActivatedMap.end())
  {
    return timesActivatedMap[mac];
  }
  else
    return 0;
}