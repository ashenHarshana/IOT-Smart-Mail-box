#include <WiFi.h>

// ---------------- WIFI CONFIG ----------------
#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

// PIR + Ultrasonic Pins
const int PIR_PIN = 27;
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;
const float DISTANCE_THRESHOLD_CM = 5.0;

int lastState = 0;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize sensors
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("=== SENSOR DEBUG STARTED ===");
  Serial.println("Waiting 30 seconds for PIR stabilization...");
  
  // PIR warm-up period
  for(int i = 30; i > 0; i--) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nSensors ready! Starting monitoring...");
  
  
}

float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 60000UL);
  
  if (duration == 0) return -1;
  
  return duration * 0.034 / 2;
}

void loop() {
  int pirValue = digitalRead(PIR_PIN);
  float distance = measureDistanceCm();
  

  if (millis() - lastPrint > 2000) {
    Serial.print("PIR: ");
    Serial.print(pirValue);
    Serial.print(" | Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    lastPrint = millis();
  }
  
  if (pirValue == HIGH) {
    if (distance > 0 && distance <= DISTANCE_THRESHOLD_CM) {
      if (lastState != 1) {
        Serial.println("✅ MOTION DETECTED IN RANGE!");
        lastState = 1;
      }
    } else {
      if (lastState != 0) {
        Serial.println("⚠️ Motion detected but OUT OF RANGE");
        lastState = 0;
      }
    }
  } else {
    if (lastState != 0) {
      Serial.println("❌ No motion detected");
      lastState = 0;
    }
  }
  
  delay(200);
}