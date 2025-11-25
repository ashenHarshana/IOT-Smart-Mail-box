#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// WiFi configaration
#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

// Pins
#define DOOR_SENSOR_PIN 4

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo myServo;

// State Variables
int lastDoorState = -1;
bool doorUnlocked = false;
bool mailRetrievalDetected = false;

// Timers
unsigned long lastRemoteCheck = 0; 

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();
  myServo.attach(SERVO_PIN);
  myServo.write(0); 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nConnected with IP:");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
   Serial.println("Firebase Connected!");

// Sync initial state

  Serial.println("System Active.");

}

void loop() {
  // 4. RFID ACCESS (With 3s Cooldown)
  if (millis() - lastRFIDScan > 3000) { 
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      byte *uid = rfid.uid.uidByte;
      String uidString = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (uid[i] < 0x10) uidString += "0";
        uidString += String(uid[i], HEX);
        if (i < rfid.uid.size - 1) uidString += " ";
      }
      uidString.toUpperCase();
      
      if (compareUID(uid, validCard1) || compareUID(uid, validCard2)) {
        if (!doorUnlocked) unlockDoor("RFID");
        else lockDoor("RFID");
        lastRFIDScan = millis(); 
      } else {
        Serial.println("Access Denied");
        Firebase.pushString(fbdo, "/logs", "Denied: " + uidString);
        lastRFIDScan = millis();
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}
