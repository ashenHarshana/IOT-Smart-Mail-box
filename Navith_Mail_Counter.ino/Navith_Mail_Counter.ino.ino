#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

#define IR_PIN 25

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int totalMailCount = 0;
int lastIrState = HIGH; 

void setup() {
  Serial.begin(115200);
  pinMode(IR_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
     delay(300); 
     Serial.print("."); }
  Serial.println("\nConnected.");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  
  Serial.println("Navith's Mail System Ready.");
}

void loop() {
  int currentIrState = digitalRead(IR_PIN);

  if (lastIrState == HIGH && currentIrState == LOW) {
    totalMailCount++; 
    Serial.println("📩 MAIL DETECTED!");
    Serial.println("Count: " + String(totalMailCount));
    
    if (Firebase.ready()) {
      Firebase.setInt(fbdo, "/mailCount", totalMailCount);
      Firebase.setString(fbdo, "/mailStatus", "New Mail Arrived");
      Firebase.pushString(fbdo, "/logs", "Mail Count: " + String(totalMailCount));
    }
    delay(500); 
  }
  lastIrState = currentIrState;
  delay(50);
}