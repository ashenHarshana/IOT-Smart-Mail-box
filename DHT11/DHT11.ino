#include "DHT.h"

#define DHTPIN 21     

// Define the type of sensor
#define DHTTYPE DHT11  

// Initialize the sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);
  Serial.println("DHT11 Sensor Test Starting...");

  dht.begin();
}

void loop() {
  delay(2000);


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor! Check wiring.");
    return; 
  }


  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");

  if (h > 75.0) {
    Serial.println("  -> ALERT: Moisture level is HIGH! ");
  }
}