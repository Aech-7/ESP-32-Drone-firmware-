#include <WiFi.h>

#define ESP_POWER_PIN 4   // IO4 = ESP32 power ON/OFF (from manual)

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Enable ESP32 power rail
  pinMode(ESP_POWER_PIN, OUTPUT);
  digitalWrite(ESP_POWER_PIN, HIGH);
  delay(200);   // allow power rail to stabilize

  Serial.println("ESP Power Enabled");

  WiFi.mode(WIFI_STA);
  delay(200);

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
