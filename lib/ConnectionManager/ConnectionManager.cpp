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
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", _mqtt_user, 32);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", _mqtt_password, 32);
  WiFiManagerParameter custom_mqtt_path("path", "location", _mqtt_path, 40);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
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

  //save the custom parameters to FS
  if (_shouldSaveConfig) {
    //read updated parameters
    strcpy(_mqtt_server, custom_mqtt_server.getValue());
    strcpy(_mqtt_port, custom_mqtt_port.getValue());
    strcpy(_mqtt_user, custom_mqtt_user.getValue());
    strcpy(_mqtt_password, custom_mqtt_password.getValue());
    strcpy(_mqtt_path, custom_mqtt_path.getValue());

    writeMqttConfiguration();
  }
}

const char* ConnectionManager::getMQTTServer() {
  return _mqtt_server;
}

const char* ConnectionManager::getMQTTPort() {
  return _mqtt_port;
}

const char* ConnectionManager::getMQTTUser() {
  return _mqtt_user;
}

const char* ConnectionManager::getMQTTPassword() {
  return _mqtt_password;
}

const char* ConnectionManager::getMQTTPath() {
  return _mqtt_path;
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
          strcpy(_mqtt_user, json["mqtt_user"]);
          strcpy(_mqtt_password, json["mqtt_password"]);
          strcpy(_mqtt_path, json["mqtt_path"]);

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
  json["mqtt_user"] = _mqtt_user;
  json["mqtt_password"] = _mqtt_password;
  json["mqtt_path"] = _mqtt_path;

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

void ConnectionManager::resetConfiguration() {
  // format FileSystem to delete custom config
  SPIFFS.format();
  // reset wifi settings
  wifiManager.resetSettings();
}

/** MQTT **/

MQTTManager::MQTTManager() {
  _clientId = "MRSensor-";
  _clientId += ESP.getChipId();
}

MQTTManager::MQTTManager(const char* clientId) {
  _clientId = clientId;
}

void MQTTManager::setConnectionManager(ConnectionManager &connectionManager) {
  _connectionManager = &connectionManager;
}

void MQTTManager::init(ConnectionManager &connectionManager) {

  setConnectionManager(connectionManager);
  PubSubClient c(_espClient);
  _pubsubClient = c;
  _pubsubClient.setServer(_connectionManager->_mqtt_server,
                          atoi(_connectionManager->_mqtt_port));
}

boolean MQTTManager::connect() {
  if(_pubsubClient.connect(_clientId,
                        _connectionManager->_mqtt_user,
                        _connectionManager->_mqtt_password)) {
    Serial.println("Connection to MQTT-Server successful.");
  } else {
      Serial.print("Connection to MQTT-Server failed, rc=");
      Serial.println(_pubsubClient.state());
  }
  return isConnected();
}

void MQTTManager::disconnect() {
  _pubsubClient.disconnect();
}

boolean MQTTManager::publish(const char* topic, const char* message) {
  return publish(topic, message, false);
}

boolean MQTTManager::publish(const char* topic, const char* message, boolean retained) {
  if (!isConnected()) {
    Serial.println("No connection to mqtt server available. Connecting...");
    if(!connect()) {
      Serial.println("Connection failed.");
      return false;
    }
  }
  return _pubsubClient.publish(topic, message, retained);
}

boolean MQTTManager::isConnected() {
  return _pubsubClient.connected();
}

boolean MQTTManager::ping() {
  return _pubsubClient.loop();
}
