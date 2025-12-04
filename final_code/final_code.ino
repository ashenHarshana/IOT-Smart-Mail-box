#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <DHT.h>

// ---------------- CONFIGURATION ----------------
#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

// Pins
#define IR_PIN          25
#define DHT_PIN         21
#define DOOR_SENSOR_PIN 4
#define PIR_PIN         27
#define TRIG_PIN        12
#define ECHO_PIN        14
#define RFID_SS_PIN     5
#define RFID_RST_PIN    22
#define SERVO_PIN       13

#define DHTTYPE DHT11
const float DISTANCE_THRESHOLD_CM = 10.0; 

// ---------------- OBJECTS ----------------
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo myServo;
DHT dht(DHT_PIN, DHTTYPE);

// Valid Cards
byte validCard1[4] = {0x83, 0x3B, 0xAA, 0xFC};
byte validCard2[4] = {0x23, 0xAD, 0x03, 0xF7};

// State Variables
int totalMailCount = 0;          
int lastIrState = HIGH;          
int lastDoorState = -1;
bool doorUnlocked = false;       
bool mailRetrievalDetected = false; 

// Timers
unsigned long lastDHTTime = 0;
unsigned long lastSecurityAlertTime = 0;
unsigned long lastDebugPrint = 0;
unsigned long lastRemoteCheck = 0; 
unsigned long lastRFIDScan = 0; 

// ---------------- FUNCTIONS ----------------

float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return 0; 
  return duration * 0.034 / 2;
}

bool compareUID(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) { if (a[i] != b[i]) return false; }
  return true;
}

// *** UPDATED UNLOCK LOGIC ***
void unlockDoor(String source) {
  if (!doorUnlocked) {
    myServo.write(180); 
    doorUnlocked = true;
    mailRetrievalDetected = false; 
    Serial.println("UNLOCKED via " + source);
    
    // Update Status
    Firebase.setString(fbdo, "/lockStatus", "Unlocked");
    Firebase.pushString(fbdo, "/logs", "Unlocked via " + source);
    
    // SYNC FIX: Tell Firebase the door is now OPEN ("1") so it doesn't auto-close
    if (source == "RFID") {
       Firebase.setString(fbdo, "/remoteUnlock", "1");
    }
  }
}

// *** UPDATED LOCK LOGIC ***
void lockDoor(String source) {
  if (doorUnlocked) {
    myServo.write(0); 
    doorUnlocked = false;
    Serial.println("LOCKED via " + source);
    
    // Update Status
    Firebase.setString(fbdo, "/lockStatus", "Locked");
    Firebase.pushString(fbdo, "/logs", "Locked via " + source);
    
    // SYNC FIX: Tell Firebase the door is now CLOSED ("0")
    if (source == "RFID") {
       Firebase.setString(fbdo, "/remoteUnlock", "0");
    }
    
    // Reset Count if needed
    if (mailRetrievalDetected == true) {
       totalMailCount = 0;
       Serial.println("RESETTING MAIL COUNT");
       Firebase.setInt(fbdo, "/mailCount", 0);
       Firebase.setString(fbdo, "/mailStatus", "Empty");
       mailRetrievalDetected = false; 
    }
  }
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- SMART MAILBOX STARTING ---");

  pinMode(IR_PIN, INPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT); 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  dht.begin();
  SPI.begin();
  rfid.PCD_Init();
  myServo.attach(SERVO_PIN);
  myServo.write(0); 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println("\nConnected: " + WiFi.localIP().toString());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  
  // Sync initial state
  Firebase.setString(fbdo, "/remoteUnlock", "0");
  
  Serial.println("System Active.");
}

// ---------------- MAIN LOOP ----------------

void loop() {
  
  // 1. REMOTE CONTROL SYNC (Every 2s)
  if (millis() - lastRemoteCheck > 2000) {
    // Only check if connected
    if (Firebase.ready()) {
      if (Firebase.getString(fbdo, "/remoteUnlock")) {
        String cmd = fbdo.stringData();
        
        // Only act if the command is DIFFERENT from current state
        // "1" = Unlock, "0" = Lock
        if (cmd == "1" && !doorUnlocked) {
          unlockDoor("Remote App");
        } 
        else if (cmd == "0" && doorUnlocked) {
          lockDoor("Remote App");
        }
      }
    }
    lastRemoteCheck = millis();
  }

  // 2. MAIL COUNTER
  int currentIrState = digitalRead(IR_PIN);
  if (lastIrState == HIGH && currentIrState == LOW) {
    totalMailCount++; 
    Serial.println("MAIL DETECTED");
    Firebase.setInt(fbdo, "/mailCount", totalMailCount);
    Firebase.setString(fbdo, "/mailStatus", "New Mail Arrived");
    Firebase.pushString(fbdo, "/logs", "Mail Count: " + String(totalMailCount));
    delay(500); 
  }
  lastIrState = currentIrState;

  // 3. DOOR SENSOR
  int currentDoorState = digitalRead(DOOR_SENSOR_PIN);
  if (currentDoorState != lastDoorState) {
    if (currentDoorState == HIGH) { 
      if (doorUnlocked) {
        Serial.println("Door Opened (Authorized)");
        Firebase.setString(fbdo, "/physicalDoor", "Open");
        mailRetrievalDetected = true;
      } else {
        Serial.println("ALARM: DOOR FORCED OPEN!");
        Firebase.setString(fbdo, "/physicalDoor", "FORCED OPEN");
        Firebase.setString(fbdo, "/security", "Door Forced Open!");
        Firebase.pushString(fbdo, "/logs", "ALARM: Door Forced!");
      }
    } else {
      Serial.println("Door Closed");
      Firebase.setString(fbdo, "/physicalDoor", "Closed");
    }
    lastDoorState = currentDoorState;
    delay(100);
  }

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

  // 5. SECURITY (PIR)
  if (millis() - lastDebugPrint > 500) {
    int pirValue = digitalRead(PIR_PIN);
    if (pirValue == HIGH) {
      float dist = measureDistanceCm();
      if (dist > 0 && dist <= DISTANCE_THRESHOLD_CM) {
        if (millis() - lastSecurityAlertTime > 5000) {
           Firebase.setString(fbdo, "/security", "Intruder Detected!");
           Firebase.pushString(fbdo, "/logs", "Security Alert!");
           lastSecurityAlertTime = millis();
        }
      }
    }
    lastDebugPrint = millis();
  }

  // 6. ENVIRONMENT
  if (millis() - lastDHTTime > 5000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      Firebase.setFloat(fbdo, "/temperature", t);
      Firebase.setFloat(fbdo, "/humidity", h);
    }
    lastDHTTime = millis();
  }
}