#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ---------------- WIFI -----------------
#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

// ---------------- FIREBASE -----------------
#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase Email/Password Login
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------------- RFID -----------------
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// ---------------- SERVO -----------------
#define SERVO_PIN 13
Servo myServo;

// ---------------- VALID CARDS -----------------
byte validCard1[4] = {0x83, 0x3B, 0xAA, 0xFC};
byte validCard2[4] = {0x23, 0xAD, 0x03, 0xF7};

// Door state
bool doorOpen = false;

// ---------------- FUNCTIONS -----------------

bool compareUID(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void sendToFirebase(String uid, String status, String doorState) {
  // Save log
  Firebase.pushString(fbdo, "/doorLock", uid + " | " + status + " | Door: " + doorState);

  // Update door status
  Firebase.setString(fbdo, "/doorStatus", doorState);
}

// ---------------- SETUP -----------------

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  // ---------- WIFI CONNECT ----------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nConnected with IP:");
  Serial.println(WiFi.localIP());

  // ---------- FIREBASE CONFIG ----------
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);

  Serial.println("Firebase Connected!");

  // ---------- SERVO ----------
  myServo.attach(SERVO_PIN);
  myServo.write(0); // start closed

  Serial.println("System Ready - Scan your RFID card...");
}

// ---------------- LOOP -----------------

void loop() {

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  byte *uid = rfid.uid.uidByte;

  // Convert UID to string
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (uid[i] < 0x10) uidString += "0";
    uidString += String(uid[i], HEX);
    if (i < rfid.uid.size - 1) uidString += " ";
  }
  uidString.toUpperCase();

  Serial.print("Scanned UID: ");
  Serial.println(uidString);

  // ---- VALID CARD ----
  if (compareUID(uid, validCard1) || compareUID(uid, validCard2)) {

    if (!doorOpen) {
      myServo.write(180);
      doorOpen = true;

      Serial.println("Access Granted - Door opened.");
      sendToFirebase(uidString, "Access Granted", "open");
    }
    else {
      myServo.write(0);
      doorOpen = false;

      Serial.println("Access Granted - Door closed.");
      sendToFirebase(uidString, "Access Granted", "closed");
    }
  }

  // ---- INVALID CARD ----
  else {
    Serial.println("Access Denied!");
    sendToFirebase(uidString, "Access Denied", "unchanged");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
