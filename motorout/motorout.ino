
int motorVal = (1600);
void setup() {
  //Serial.begin(9600);
  //while(!Serial);
  setupMotor();
}

void loop() {
  //Serial.println(motorVal);
  setMotor();
}

void setupMotor() {
  pinMode(9, OUTPUT);
  TCCR1A |= (1<<WGM11); TCCR1A &= ~(1<<WGM10); TCCR1B |= (1<<WGM13);
  TCCR1B &= ~(1<<CS11);
  ICR1 |= 0x3FFF;
  TCCR1A |= _BV(COM1A1);
  OCR1A = 1000;
}

void setMotor() {
  OCR1A = motorVal;
}
