
void setupMotor() {
  //http://www.avrbeginners.net/architecture/timers/timers.html#16bit_timer
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  TCCR1A |= (1<<WGM11);  //Setting Timer comparator 1A register bit 1 (WGM11) to on
  TCCR1A &= ~(1<<WGM10); //Setting Timer comparator 1A register bit 0 (WGM10) to off
  TCCR1B &= ~(1<<WGM12) &  ~(1<<CS11) & ~(1<<CS12); //Turn off WGM12, CS11, CS12, 
  TCCR1B |= (1<<WGM13) | (1<<CS10); //Turn on WGM13 and CS10
  ICR1 |= 0x3FFF;        //Setting Input Capture register to high
  TCCR1A |= (1<<COM1A1); //Pin 9 connected to comparator 1A
  TCCR1A |= (1<<COM1B1); //Pin 10 connected to comparator 1B
  OCR1A |= motorValue[0]<<3;     //Setting Timer comparator output register to zero percent throttle.
  OCR1B |= motorValue[0]<<3;
  // Second Timer
  TCCR3A |= (1<<WGM31); //Setting Timer comparator 3A register bit 1 (WGM31) to on
  TCCR3A &= ~(1<<WGM30); //Setting Timer comparator 3A register bit 0 (WGM32) to off
  TCCR3B &= ~(1<<WGM32) &  ~(1<<CS31) & ~(1<<CS32); //Setting Timer comparator 3B register bit 2 (WGM32), 1 (CS31) and 2 (CS32) to off
  TCCR3B |= (1<<WGM33) | (1<<CS30); //Setting Timer comparator 3B register bit 4 (WGM32) and 0 (CS30) to on
  ICR3   |= 0x3FFF; // TOP to 16383;
  TCCR3A |= (1<<COM3A1); // connect pin 5 to timer 3 channel A
  TCCR4E |= (1<<ENHC4); //Setting Timer comparator 4E register bit ? (ENHC4) to on
  TCCR4B &= ~(1<<CS41); //Setting Timer comparator 4B register bit 1 (CS41) to off
  TCCR4B |= (1<<CS42)|(1<<CS40); //Setting Timer comparator 4B register bit 0 (CS40) and 2 (CS42) to on
  TCCR4D |= (1<<WGM40); //Setting Timer comparator 4D register bit ? (WGM40) to on
  TC4H = 0x3; //
  OCR4C = 0xFF;//
  TCCR4C |= (1<<COM4D1)|(1<<PWM4D);
  OCR3A = motorValue[0]<<3;
  TC4H = motorValue[0]>>8;
  OCR4D = (motorValue[0]&0xFF);
  
  delay(500);
}

void setMotor() {
  OCR1A = motorValue[0]<<3;
  OCR1B = motorValue[1]<<3;
  OCR3A = motorValue[2]<<3;
  TC4H = motorValue[3]>>8;
  OCR4D = (motorValue[3]&0xFF);
  //if (counter > 12000)
    //motorVal = 2000<<3;
}
