#include <string>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

#include "HardwareId.h"
#include "Firmware.h"

#include <CloudSync.h>
#include <peripherals/Led.h>
#include <peripherals/Button.h>
#include <peripherals/WiFiButton.h>

ESP8266WiFiMulti wifiMulti;
BearSSL::WiFiClientSecure client;
CloudSync *cloudSync = &CloudSync::getInstance();

Led led(0);
Button button(2, []
              { led.toggle(); });
WiFiButton wifiButton({0x84, 0xf3, 0xeb, 0xc9, 0x27, 0x4a}, []
                      { led.toggle(); });

void setup()
{
  Serial.begin(115200);
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

  cloudSync->begin(wifiMulti, client);
}

void loop()
{
  cloudSync->run();
  Serial.print("Free Memory: " + String(ESP.getFreeHeap()) + " B\r");
  delay(10);
}