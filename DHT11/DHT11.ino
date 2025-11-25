#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

#define WIFI_SSID "OPPO A78"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyDCf1MiexBrF9hViJDh4nhiEQBhRUqPAkk"
#define DATABASE_URL "test-2-ee62f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "ashenharshana02@gmail.com"
#define USER_PASSWORD "Ashen@fdo10"

#define DHT_PIN 21
#define DHTTYPE DHT11

DHT dht(DHT_PIN, DHTTYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println("\nConnected.");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Serial.println("Ridma's System Ready.");
}

void loop() {
  delay(5000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Serial.print("Temp: "); Serial.println(t);
    Firebase.setFloat(fbdo, "/temperature", t);
    Firebase.setFloat(fbdo, "/humidity", h);
  } else {
    Serial.println("DHT Error");
  }
}