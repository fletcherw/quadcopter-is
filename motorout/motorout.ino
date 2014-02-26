
int motorVal = (1000<<3);
int counter = 0;
void setup() {
  delay(500);
  setupMotor();
}

void loop() {
  //Serial.println(motorVal);
  setMotor();
}

void setupMotor() {
  //http://www.avrbeginners.net/architecture/timers/timers.html#16bit_timer
  pinMode(9, OUTPUT);
  TCCR1A |= (1<<WGM11);  //Setting Timer comparator 1A register bit 1 (WGM11) to on
  TCCR1A &= ~(1<<WGM10); //Setting Timer comparator 1A register bit 0 (WGM10) to off
  TCCR1B &= ~(1<<WGM12) &  ~(1<<CS11) & ~(1<<CS12); //Turn off WGM12, CS11, CS12, 
  TCCR1B |= (1<<WGM13) | (1<<CS10); //Turn on WGM13 and CS10
 
  ICR1 |= 0x3FFF;        //Setting Input Capture register to high
  TCCR1A |= (1<<COM1A1); //Pin 9, Setting Timer comparator 1A register bit 7 (WGM11) to on
  OCR1A |= 1200<<3;     //Setting Timer comparator output register to zero percent throttle.
  delay(500);
}

void setMotor() {
  counter++;
  delay(500);
  OCR1A = motorVal;
  if (counter > 10) {
    if (motorVal < (1600<<3)) {
      motorVal = (1600<<3);
    } else if (motorVal > (1000<<3)){
      motorVal =(1000<<3);
    }
    
    counter = 0;
  }
  //if (counter > 12000)
    //motorVal = 2000<<3;
}
