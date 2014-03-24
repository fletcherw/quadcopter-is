
static int16_t motorValue[4];

void setup() {
  motorValue[0] = 1200;
  Serial.begin(9600);
  while(!Serial);
  delay(500);
  setupMotor();
  imuSetup();
  //setupReceiver();
  delay(500);
 
  //imuLoop();
}

void loop() {
  imuLoop();
  setMotor();
}
