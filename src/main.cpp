#include <Arduino.h>
#include <Preferences.h>
#include "../inc/PinDefinitionsAndMore.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <IRremote.hpp>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "DNSServer.h"
#include "ESPmDNS.h"
#include "../inc/globalDefines.h"
#include "../inc/tasks.h"
#include "../inc/webServer.h"
#include "../inc/wifi.h"

// Method to initialize the filesystem (SPIFFS)
void InitializeSPIFFS() {
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void setup() {
  InitializeSPIFFS();
  connectWifi(wifiSSID.c_str(), wifiPass.c_str());
  Serial.begin(115200);

  //structure to hold received IR data
  prefs.begin("data");
  prefs.begin("buffer");
  prefs.begin("brightness1");
  prefs.begin("brightness2");
  prefs.begin("brightness3");
  size_t size = prefs.getBytesLength("data");
  prefs.getBytes("buffer", &bufferMinutes, sizeof(bufferMinutes));
  prefs.getBytes("brightness1", &brightness1, sizeof(brightness1));
  prefs.getBytes("brightness2", &brightness2, sizeof(brightness2));
  prefs.getBytes("brightness3", &brightness3, sizeof(brightness3));
  Serial.println(bufferMinutes);
  prefs.getBytes("data", &dataFromStorage, size);
  sStoredIRData = dataFromStorage;

  InitializeWebServer();

  // Initialize pins
  pinMode(BTN_BRIGHTNESS_DOWN, INPUT);
  pinMode(BTN_BRIGHTNESS_UP, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  // Tasks
  xTaskCreate(TaskLightOn, "LightOn", 2048, NULL, 2, NULL);
  xTaskCreate(TaskLightOff,"LightOff", 2048, NULL, 2, NULL);
  xTaskCreate(TaskLightBrightnessUP,"BrightnessUP", 2048, NULL, 2, NULL);
  xTaskCreate(TaskLightBrightnessDOWN,"BrightnessDOWN", 2048, NULL, 2, NULL);
  xTaskCreate(TaskUDP,"UDP", 2048, NULL, 3, NULL); // requires 2kb stack size to work
  xTaskCreate(TaskStandUpDimming, "Stand up dimming", 2048, NULL, 2, NULL);
  xTaskCreate(IrSendTask, "IRSend", 1024, NULL, 4, NULL); 
  xTaskCreate(IrReceiveTask, "IRReceive", 2048, NULL, 2, NULL); 
  xTaskCreate(IrReceiveButtonTask, "IRReceiveTrigger", 1024, NULL, 3, NULL);
  xTaskCreate(sittingLogicTask, "SittingLogic", 2048, NULL, 2, NULL);
  xTaskCreate(ControlLedsTask, "ControlLeds", 2048, NULL, 2, NULL);
}

void loop() {
  dnsServer.processNextRequest();
}
