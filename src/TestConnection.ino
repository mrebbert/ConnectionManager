#include <ConnectionManager.h>

ConnectionManager connectionManager;

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println();
  //connectionManager.resetConfiguration();
  connectionManager.init();

  Serial.printf("MQTT Server: %s\n", connectionManager.getMQTTServer());
  Serial.printf("MQTT Port: %s\n", connectionManager.getMQTTPort());
  Serial.printf("MQTT Password: %s\n", connectionManager.getMQTTPassword());
  Serial.printf("MQTT Path: %s\n", connectionManager.getMQTTPath());
}

void loop() {
}
