#include <ConnectionManager.h>

ConnectionManager *currentObject;

//callback notifying us of the need to save config
void ConnectionManager::saveConfigCallback() {
  Serial.println("save config callback raised.");
  _shouldSaveConfig = true;
}

// static wrapper to call the callback method within a library.
static void callbackWrapper() {
  currentObject->saveConfigCallback();
}

/*
Constructor
*/
ConnectionManager::ConnectionManager() {
  currentObject = this;
}

void ConnectionManager::init() {
  //set config save notify callback
  wifiManager.setSaveConfigCallback(callbackWrapper);
  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(10);
  //sets timeout until configuration portal gets turned off useful to make
  //it all retry or go to sleep. In seconds:
  //wifiManager.setTimeout(120);
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", _mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", _mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_password("password", "password", _mqtt_password, 32);
  WiFiManagerParameter custom_mqtt_path("path", "location", _mqtt_path, 40);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.addParameter(&custom_mqtt_path);

  readMqttConfiguration();

  String ssid = apSSIDPrefix + String(ESP.getChipId()).substring(0,22);
  if (!wifiManager.autoConnect(ssid.c_str(), apPassword)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected.");

  //read updated parameters
  strcpy(_mqtt_server, custom_mqtt_server.getValue());
  strcpy(_mqtt_port, custom_mqtt_port.getValue());
  strcpy(_mqtt_password, custom_mqtt_password.getValue());
  strcpy(_mqtt_path, custom_mqtt_path.getValue());

  //save the custom parameters to FS
  if (_shouldSaveConfig) {
    writeMqttConfiguration();
  }
}

/*
  Reads the additional mqtt configuration from the json file to attributes.
*/
void ConnectionManager::readMqttConfiguration() {
  if (SPIFFS.begin()) {
    Serial.println("file system mounted.");

    if (SPIFFS.exists(configFile)) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File fileHandle = SPIFFS.open(configFile, "r");

      if (fileHandle) {
        Serial.println("opened config file");
        size_t size = fileHandle.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        fileHandle.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(_mqtt_server, json["mqtt_server"]);
          strcpy(_mqtt_port, json["mqtt_port"]);
          strcpy(_mqtt_password, json["password"]);
          strcpy(_mqtt_path, json["path"]);
          /*
          Serial.println("**** Variables after strcpy: ");
          Serial.print("MQTT Server: ");
          Serial.println(_mqtt_server);
          Serial.print("MQTT Port: ");
          Serial.println(_mqtt_port);
          Serial.print("MQTT Password: ");
          Serial.println(_mqtt_password);
          Serial.print("MQTT Path: ");
          Serial.println(_mqtt_path);
          */
        } else {
          Serial.println("failed to load json config");
        }
      }
      fileHandle.close();
    }
    SPIFFS.end();
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void ConnectionManager::writeMqttConfiguration() {
  Serial.println("Writing config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = _mqtt_server;
  json["mqtt_port"] = _mqtt_port;
  json["password"] = _mqtt_password;
  json["path"] = _mqtt_path;

  if (SPIFFS.begin()) {
    File fileHandle = SPIFFS.open(configFile, "w");
    if (!fileHandle) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(fileHandle);
    fileHandle.close();
    SPIFFS.end();
  } else {
    Serial.println("failed to mount FS");
  }
  //end save
}

std::string ConnectionManager::getMQTTServer() {
  std::string _str = _mqtt_server;
  Serial.print("*** MQTT Server: ");
  Serial.println(_mqtt_server);
  Serial.printf("Returning MQTT Server: %s\n", _str.c_str());
  return _str;
}

std::string ConnectionManager::getMQTTPort() {
  std::string _str = _mqtt_port;
  Serial.printf("Returning MQTT Port: %s\n", _str.c_str());
  return _str;
}

std::string ConnectionManager::getMQTTPassword() {
  std::string _str = _mqtt_password;
  Serial.printf("Returning MQTT Password: %s\n", _str.c_str());
  return _str;
}

std::string ConnectionManager::getMQTTPath() {
  std::string _str = _mqtt_path;
  Serial.printf("Returning MQTT Path: %s\n", _str.c_str());
  return _str;
}

void ConnectionManager::resetConfiguration() {
  // format FileSystem to delete custom config
  SPIFFS.format();
  // reset wifi settings
  wifiManager.resetSettings();
}
