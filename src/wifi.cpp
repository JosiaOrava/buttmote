#include "../inc/wifi.h"
#include "WiFiUdp.h"

void connectWifi(const char *SSID, const char *PASSWORD){
  Serial.println("Connecting to: " + String(SSID));

  // Delete old config
  WiFi.disconnect(true);
  // event handler
  WiFi.onEvent(WiFiEvent);

  // ini connection
  WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());

  Serial.println("Starting connection...");
}

void WiFiEvent(WiFiEvent_t event){
  switch(event){
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      // Connection succesfull
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());

      // ini udp
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Connection lost");
      connected = false;
      break;

    default: break;
  }
}