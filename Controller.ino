#include <WiFi.h>
#include <esp_now.h>
#include <math.h>
#include "driver/gpio.h"

#define BUZZER_PIN GPIO_NUM_15

uint8_t droneMAC[] = {0x68, 0x25, 0xDD, 0xCC, 0xAD, 0x28}; //MAC address

// Mode-2 mapping (standard)
#define ROLL_PIN      36   // Right stick X
#define PITCH_PIN     39   // Right stick Y
#define YAW_PIN       35   // Left stick X
#define THROTTLE_PIN  34   // Left stick Y

// -------- ROLL -------- thrttle
#define ROLL_MIN      0
#define ROLL_CENTER   1958
#define ROLL_MAX      4095

// -------- PITCH ------- yaw
#define PITCH_MIN     446
#define PITCH_CENTER  1950
#define PITCH_MAX     4095

// -------- YAW ---------  pitch
#define YAW_MIN       0
#define YAW_CENTER    1937
#define YAW_MAX       4095

// -------- THROTTLE ---- roll
#define THR_MIN       585
#define THR_MAX       4095

//limits
#define MAX_ROLL_ANGLE    20.0f    // degrees
#define MAX_PITCH_ANGLE   20.0f    // degrees
#define MAX_YAW_RATE      90.0f    // deg/sec
#define DEADZONE          0.05f    // 5%

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

ControlValue txCmd;

// Calibrated normalization (-1 to +1)
float normalizeCalibrated(int adc, int minVal, int centerVal, int maxVal) {
  if (adc > centerVal) {
    return (float)(adc - centerVal) / (float)(maxVal - centerVal);
  } else {
    return (float)(adc - centerVal) / (float)(centerVal - minVal);
  }
}

// Deadzone
float applyDeadzone(float v) {
  if (fabs(v) < DEADZONE) return 0.0f;
  return constrain(v, -1.0f, 1.0f);
}

// Throttle mapping (0 → 1)
float mapThrottle(int adc, int minVal, int maxVal) {
  float t = (float)(adc - minVal) / (float)(maxVal - minVal);
  return constrain(t, 0.0f, 1.0f);
}

void setup() {
  Serial.begin(115200);

  analogSetAttenuation(ADC_11db);

  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  gpio_pulldown_en(BUZZER_PIN);
  gpio_hold_en(BUZZER_PIN);   // critical

  delay(2000);

  // WiFi in station mode
  WiFi.mode(WIFI_STA);
  delay(200);

  Serial.print("Remote MAC: ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1);
  }

  // Add drone as peer
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, droneMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (1);
  }

  Serial.println("Joystick transmitter ready");
}

void loop() {
  digitalWrite(15, LOW);

  // -------- Read ADCs --------
  int rollADC     = analogRead(ROLL_PIN);
  int pitchADC    = analogRead(PITCH_PIN);
  int yawADC      = analogRead(YAW_PIN);
  int throttleADC = analogRead(THROTTLE_PIN);

  // -------- Normalize --------
  float rollNorm  = applyDeadzone(
                      normalizeCalibrated(rollADC,
                                           ROLL_MIN,
                                           ROLL_CENTER,
                                           ROLL_MAX));

  float pitchNorm = applyDeadzone(
                      normalizeCalibrated(pitchADC,
                                           PITCH_MIN,
                                           PITCH_CENTER,
                                           PITCH_MAX));

  float yawNorm   = applyDeadzone(
                      normalizeCalibrated(yawADC,
                                           YAW_MIN,
                                           YAW_CENTER,
                                           YAW_MAX));

  float throttle  = mapThrottle(throttleADC,
                                THR_MIN,
                                THR_MAX);

  // -------- Fill packet --------
  txCmd.mode = STABLIZE;
  txCmd.targetRollAngle  = rollNorm  * MAX_ROLL_ANGLE;
  txCmd.targetPitchAngle = pitchNorm * MAX_PITCH_ANGLE;
  txCmd.targetYawAngle   = yawNorm   * MAX_YAW_RATE;
  txCmd.targetThrottle   = throttle;
  txCmd.timestamp        = millis();

  // -------- Send packet --------
  esp_now_send(droneMAC, (uint8_t*)&txCmd, sizeof(txCmd));

  Serial.print("T:");
  Serial.print(txCmd.targetThrottle, 2);
  Serial.print("  R:");
  Serial.print(txCmd.targetRollAngle, 1);
  Serial.print("  P:");
  Serial.print(txCmd.targetPitchAngle, 1);
  Serial.print("  Y:");
  Serial.println(txCmd.targetYawAngle, 1);

  delay(20);   // 50 Hz RC update rate
}
