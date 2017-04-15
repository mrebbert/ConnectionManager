#ifndef ConnectionManager_h
#define ConnectionManager_h

#include <FS.h>                 //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library
#include <DNSServer.h>          //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>   //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>        //https://github.com/bblanchon/ArduinoJson

class ConnectionManager {
public:

  ConnectionManager();
  void resetConfiguration();
  void init();
  std::string getMQTTServer();
  std::string getMQTTPort();
  std::string getMQTTPassword();
  std::string getMQTTPath();

  void saveConfigCallback();
private:
  WiFiManager wifiManager;
  bool _shouldSaveConfig   = false;
  const char *configFile   = "/config.json";
  const char *apSSIDPrefix = "MRSensor-";
  const char *apPassword   = "MRSensor";
  char _mqtt_server[40];
  char _mqtt_port[6];
  char _mqtt_password[34];
  char _mqtt_path[40];

  void readMqttConfiguration();
  void writeMqttConfiguration();
};
#endif
