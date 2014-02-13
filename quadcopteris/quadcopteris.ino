
void setup() {
  Serial.begin(9600);
  while(!Serial);
  setupMotor();
  //setupReceiver();
}

void loop() {
  setMotor();
}
