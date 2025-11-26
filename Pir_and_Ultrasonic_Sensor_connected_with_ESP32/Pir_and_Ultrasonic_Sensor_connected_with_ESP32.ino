#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

#define PIR_PIN 27
#define TRIG_PIN 12
#define ECHO_PIN 14
const float DISTANCE_THRESHOLD_CM = 10.0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long lastAlert = 0;

float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return 0;
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println("\nConnected.");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);

  Serial.println("Stabilizing PIR...");
  delay(5000);
  Serial.println("Amasha's System Ready.");
}

void loop() {
  int pirValue = digitalRead(PIR_PIN);
  
  if (pirValue == HIGH) {
    float dist = measureDistanceCm();
    Serial.print("Motion! Dist: "); Serial.println(dist);

    if (dist > 0 && dist <= DISTANCE_THRESHOLD_CM) {
      if (millis() - lastAlert > 5000) {
         Serial.println("🚨 INTRUDER!");
         Firebase.setString(fbdo, "/security", "Intruder Detected!");
         Firebase.pushString(fbdo, "/logs", "Security Alert!");
         lastAlert = millis();
      }
    }
  }
  delay(500);
}