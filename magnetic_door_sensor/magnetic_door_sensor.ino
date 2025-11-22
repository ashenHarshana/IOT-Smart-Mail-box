const int sensorPin = 4; 
int sensorState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
}

void loop() {
  sensorState = digitalRead(sensorPin);

  if (sensorState == LOW) {
    Serial.println("Door is Closed");
  } else {
    Serial.println("Door is Open");
  }

  delay(100);
}
