volatile uint16_t thrustShared;
volatile uint8_t updatedShared;
volatile uint32_t thrustStart;

uint16_t thrust;

#define PIN 7
#define OUTPIN 13

void setup() {
  updatedShared = 0;
  
  
  pinMode(OUTPIN, OUTPUT);
  attachInterrupt(PIN, calcThrust, CHANGE);
}

void loop() {
  if (updatedShared) {
    thrust = thrustShared;
    updatedShared = 0;
  }
  
  digitalWrite(OUTPIN, HIGH);
  delay(thrust);
  digitalWrite(OUTPIN, LOW);
  delay(thrust);
}

void calcThrust() {
  if (digitalRead(PIN) == HIGH) {
    thrustStart = micros();
  }
  else {
      thrustShared = (uint16_t)(micros() - thrustStart);
      updatedShared = 1;
  }
}
