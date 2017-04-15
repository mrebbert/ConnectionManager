#include <ConnectionManager.h>

ConnectionManager connectionManager;

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println();

  //connectionManager.resetConfiguration();
  connectionManager.init();

  Serial.printf("MQTT Server: %s\n", connectionManager.getMQTTServer().c_str());
  Serial.printf("MQTT Port: %s\n", connectionManager.getMQTTPort().c_str());
  Serial.printf("MQTT Password: %s\n", connectionManager.getMQTTPassword().c_str());
  Serial.printf("MQTT Path: %s\n", connectionManager.getMQTTPath().c_str());
}

void loop() {
}
