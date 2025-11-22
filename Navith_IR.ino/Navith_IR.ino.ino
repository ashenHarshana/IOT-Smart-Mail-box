
const int IR_SENSOR_PIN = 25; 

void setup() {
  Serial.begin(115200); 
  Serial.println("ESP32 IR Reflective Sensor Test Started...");
  pinMode(IR_SENSOR_PIN, INPUT);
}

void loop() {
  int sensorState = digitalRead(IR_SENSOR_PIN);
  if (sensorState == LOW) {
    Serial.println("MAIL DETECTED! (Object reflected the signal)");
  } else {
    Serial.println("Slot Clear.");
  }

  delay(100); 
}