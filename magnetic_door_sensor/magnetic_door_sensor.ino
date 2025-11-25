#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>


#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

#define DOOR_SENSOR_PIN 4

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo myServo;


int lastDoorState = -1;
bool doorUnlocked = false;
bool mailRetrievalDetected = false;


unsigned long lastRemoteCheck = 0; 

void setup() {
  Serial.begin(115200);
  
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  myServo.attach(servo_pin);


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


  Serial.println("System Active.");

}

void loop() {
  
  if (millis() - lastRemoteCheck > 2000) {
    
    if (Firebase.ready()) {
      if (Firebase.getString(fbdo, "/remoteUnlock")) {
        String cmd = fbdo.stringData();
        
        
        
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
}