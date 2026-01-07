#include <WiFi.h>
#include <esp_now.h>
#include <string.h>


#define ESP_POWER_PIN 4   // power control

typedef enum {
  OFF = 0,
  MANUAL,
  CALIBERATING,
  STABLIZE,
  RTH
} FlightMode;

typedef struct {
  FlightMode mode;
  float targetThrottle;
  float targetYawAngle;
  float targetRollAngle;
  float targetPitchAngle;
  uint32_t timestamp;
} ControlValue;

volatile ControlValue rxCmd;


void onReceive(const esp_now_recv_info_t *info,
               const uint8_t *incomingData,
               int len) {

  if (len != sizeof(ControlValue)) {
    Serial.println("Invalid packet size");
    return;
  }

  memcpy((void*)&rxCmd, incomingData, sizeof(ControlValue));

  Serial.print("T:");
  Serial.print(rxCmd.targetThrottle, 2);
  Serial.print(" R:");
  Serial.print(rxCmd.targetRollAngle, 1);
  Serial.print(" P:");
  Serial.print(rxCmd.targetPitchAngle, 1);
  Serial.print(" Y:");
  Serial.println(rxCmd.targetYawAngle, 1);
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
