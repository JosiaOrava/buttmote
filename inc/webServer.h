#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>

void InitializeWebServer();
void handleAPIStatus(AsyncWebServerRequest *request);
void handleAPIWiFiStatus(AsyncWebServerRequest *request);
void handleAPISettings(AsyncWebServerRequest *request, JsonVariant &json);
void handleAPIWiFiSettings(AsyncWebServerRequest *request, JsonVariant &json);
void handleRoot(AsyncWebServerRequest *request);
void InitializeCaptivePortal();
void InitializeAccessPoint();
void InitializeDNS();

#endif
