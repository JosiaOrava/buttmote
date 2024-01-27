#include "../inc/globalDefines.h"
#include "../inc/webServer.h"

// Method to handle the api/status path
void handleAPIStatus(AsyncWebServerRequest *request){
  // Create a new JSON document
  DynamicJsonDocument json(1024);
  char buffer[1024];

  // Populate the JSON with data
  json["sitting"] = sitting;
  json["remoteEnabled"] = remoteEnabled;
  json["minutesSat"] = minutesSat;
  json["minutesStood"] = minutesStood;
  json["bufferMinutes"] = bufferMinutes;

  // Serialize the JSON to a string
  serializeJson(json, buffer);

  // Send the JSON as a response
  request->send(200, "application/json", buffer);
}
// Method to handle the /api/wifistatus path
void handleAPIWiFiStatus(AsyncWebServerRequest *request){
  Serial.println("GET request on /api/wifistatus");
  // Create a new JSON document
  DynamicJsonDocument json(1024);
  char buffer[1024];

  // Populate the JSON with data
  json["wifSSID"] = wifiSSID;
  json["bulbIP"] = bulbIP;

  // Serialize the JSON to a string
  serializeJson(json, buffer);

  // Send the JSON as a response
  request->send(200, "application/json", buffer);
  Serial.print("Response: ");
  Serial.println(buffer);
}
// Method to handle the api/status path
void handleAPISettings(AsyncWebServerRequest *request, JsonVariant &json) {
  Serial.println("POST request on /api/settings");
  // Check if the required settings are updated
  if(!json.containsKey("bufferMinutes") || !json.containsKey("remoteEnabled")) {
    // If not then respond with an error
    request->send(400, "application/json", "{\"error\":true,\"msg\":\"Missing parameters!\"}");
    Serial.println("Response: {\"error\":true,\"msg\":\"Missing parameters!\"}");
  }
  // Check if the required settings are in the correct format
  if(!json["bufferMinutes"].is<float>() || !json["remoteEnabled"].is<bool>()) {
    // If not then respond with an error
    request->send(400, "application/json", "{\"error\":true,\"msg\":\"Parameters are of wrong type!\"}");
    Serial.println("Response: {\"error\":true,\"msg\":\"Parameters are of wrong type!\"}");
  }
  // Update the variables based on the received settings
  bufferMinutes = json["bufferMinutes"];
  Serial.println(bufferMinutes);
  prefs.putBytes("buffer", &bufferMinutes, sizeof(bufferMinutes));
  float tmp;
  prefs.getBytes("buffer", &tmp, sizeof(tmp));
  Serial.println(tmp);
  remoteEnabled = json["remoteEnabled"];

  // Respond that updating was successful
  request->send(200, "application/json", "{\"error\":false,\"msg\":\"OK\"}");
  Serial.println("Response: {\"error\":false,\"msg\":\"OK\"}");
}


// Method to handle the /api/wifisettings path
void handleAPIWiFiSettings(AsyncWebServerRequest *request, JsonVariant &json) {
  Serial.println("POST request on /api/wifisettings");
  // Check if the required settings are updated
  if(!json.containsKey("wifiSSID") || !json.containsKey("wifiPass") || !json.containsKey("bulbIP")) {
    // If not then respond with an error
    request->send(400, "application/json", "{\"error\":true,\"msg\":\"Missing parameters!\"}");
    Serial.println("Response: {\"error\":true,\"msg\":\"Missing parameters!\"}");
  }
  // Check if the required settings are in the correct format
  if(!json["wifiSSID"].is<const char*>() || !json["wifiPass"].is<const char*>() || !json["bulbIP"].is<const char*>()) {
    // If not then respond with an error
    request->send(400, "application/json", "{\"error\":true,\"msg\":\"Parameters are of wrong type!\"}");
    Serial.println("Response: {\"error\":true,\"msg\":\"Parameters are of wrong type!\"}");
  }

  // Update the variables based on the received settings
  wifiSSID = json["wifiSSID"].as<String>();
  wifiPass = json["wifiPass"].as<String>();
  bulbIP = json["bulbIP"].as<String>();

  // Respond that updating was successful
  request->send(200, "application/json", "{\"error\":false,\"msg\":\"OK\"}");
  Serial.println("Response: {\"error\":false,\"msg\":\"OK\"}");
}

void handleRoot(AsyncWebServerRequest *request) {
  Serial.println("GET request on /");
  // Redirect to the wifi settings page if we don't have Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Redirecting to ");
    const String url = "http://"+WiFi.softAPIP().toString()+"/wifi";
    Serial.println(url);
    request->redirect(url);
  }
  // Serve the normal page if we have wifi
  else request->send(SPIFFS, "/index.html", "text/html");
}
// Method to initialize the web server
void InitializeWebServer() {
  // Create AsyncWebServer object on port 80

  // Route for root / web page
  server.on("/", HTTP_GET, handleRoot);
  server.on("/generate_204", HTTP_GET, handleRoot);

  // Route to serve the wifi settings page
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("GET request on /wifi");
    request->send(SPIFFS, "/wifi.html", "text/html");
  });

  // Routes to serve static WebUI files
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/main.js", SPIFFS, "/main.js");
  server.serveStatic("/wifi.js", SPIFFS, "/wifi.js");

  // Routes to serve dynamic data via the API
  server.on("/api/status", HTTP_GET, handleAPIStatus);
  static AsyncCallbackJsonWebHandler* const apiSettingsHandler = new AsyncCallbackJsonWebHandler("/api/settings", handleAPISettings);
  server.addHandler(apiSettingsHandler);

  // Routes for setting up WiFi
  server.on("/api/wifistatus", HTTP_GET, handleAPIWiFiStatus);
  static AsyncCallbackJsonWebHandler* const apiWiFiSettingsHandler = new AsyncCallbackJsonWebHandler("/api/wifisettings", handleAPIWiFiSettings);
  server.addHandler(apiWiFiSettingsHandler);

  // Start the server
  server.begin();
  Serial.println("Web server initialized.");
}

void InitializeDNS() {
  dnsServer.start(53, "esp-ap", WiFi.softAPIP());
  dnsServer.start(53, "*", WiFi.softAPIP());
}

void InitializeAccessPoint() {
  WiFi.softAP(AP_SSID, AP_PASS);
  const IPAddress AP_IP = WiFi.softAPIP();
  Serial.print("Access Point IP address: ");
  Serial.println(AP_IP);
}

void InitializeCaptivePortal() {
  InitializeAccessPoint();
  InitializeDNS();
}
