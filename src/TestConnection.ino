#include <ConnectionManager.h>

ConnectionManager connectionManager;
MQTTManager mqtt;

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println();
  //connectionManager.resetConfiguration();
  connectionManager.init();
  mqtt.init(connectionManager);

  Serial.printf("MQTT Server: %s\n", connectionManager.getMQTTServer());
  Serial.printf("MQTT Port: %s\n", connectionManager.getMQTTPort());
  Serial.printf("MQTT User: %s\n", connectionManager.getMQTTUser());
  Serial.printf("MQTT Password: %s\n", connectionManager.getMQTTPassword());
  Serial.printf("MQTT Path: %s\n", connectionManager.getMQTTPath());

  if (!mqtt.connect()) {
    Serial.println("MQTT Connection failed.");
  }
  if (!mqtt.publish("TestingMQTT", "Startup in setup() method.", false)) {
    Serial.println("MQTT publishing in setup() failed.");
  }
}

void loop() {
  if (!mqtt.isConnected()) {
    mqtt.connect();
  }

  if (!mqtt.publish("TestingMQTT", "Publish in the loop().", false)) {
    Serial.println("MQTT publishing in loop() failed.");
  }

  delay (10000);
}
