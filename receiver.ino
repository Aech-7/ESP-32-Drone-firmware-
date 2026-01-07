#include <WiFi.h>
#include <esp_now.h>

#define ESP_POWER_PIN 4   / power control

typedef struct {
  uint32_t counter;
} TestPacket;

TestPacket rxData;

void onReceive(const esp_now_recv_info_t *info,
               const uint8_t *incomingData,
               int len) {

  memcpy(&rxData, incomingData, sizeof(rxData));

  Serial.print("Packet received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Counter = ");
  Serial.println(rxData.counter);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(ESP_POWER_PIN, OUTPUT);
  digitalWrite(ESP_POWER_PIN, HIGH);
  delay(200);

  WiFi.mode(WIFI_STA);
  delay(200);

  Serial.print("Drone MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("ESP-NOW Receiver Ready");
}

void loop() {}
