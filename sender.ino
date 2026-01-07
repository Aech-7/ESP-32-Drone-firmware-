#include <WiFi.h>
#include <esp_now.h>
#include "driver/gpio.h"   // REQUIRED for gpio_* functions

// DRONE MAC (VERY IMPORTANT)
uint8_t droneMAC[] = {0x68, 0x25, 0xDD, 0xCC, 0xAD, 0x28};

#define BUZZER_PIN GPIO_NUM_15

typedef struct {
  uint32_t counter;
} TestPacket;

TestPacket txData;
uint32_t count = 0;

void setup() {
  // BUZZER CONTROL MUST BE FIRST
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  gpio_pulldown_en(BUZZER_PIN);
  gpio_hold_en(BUZZER_PIN);   // prevents WiFi/RF glitches

  Serial.begin(115200);
  delay(2000);

  // WiFi AFTER buzzer is locked LOW
  WiFi.mode(WIFI_STA);
  delay(200);

  Serial.print("Remote MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, droneMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;

  esp_now_add_peer(&peer);

  Serial.println("ESP-NOW Sender Ready");
}

void loop() {
  // keep buzzer LOW
  digitalWrite(15, LOW);

  txData.counter = count++;
  esp_now_send(droneMAC, (uint8_t *)&txData, sizeof(txData));

  Serial.println("Packet sent");
  delay(500);
}
