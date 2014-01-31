uint8_t lastPin;
#define ALL_BITS (1<<0), (1<<2),(1<<3),(1<<1),(1<<4) // 000010, 000100, 001000, 010000, 100000

static uint8_t channelBits[5] = {ALL_BITS};
volatile uint16_t channels[5] = {1500, 1500, 1500, 1500 , 1500}; // uSeconds for each channel
uint16_t freeChannels[5] = {1500, 1500, 1500, 1500, 1500}; // non volatile version
void setup() {
  sei();
  Serial.begin(9600); // start up the serial connection
  for (int ii = 2; ii <= 13; ii++) { // turn all pins to input and HIGH so when we drop them to ground they are different
    pinMode(ii, INPUT);
    digitalWrite(ii, HIGH);
  }
  PCICR |= (1 << PCIE0); // turn on interrupts for pins pb 1-5 (arduino 8 - 13)
  PCMSK0 |= (1 << PCINT4); // turn on all of those pins interrupts
  PCMSK0 |= (1 << PCINT3);
  PCMSK0 |= (1 << PCINT2);
  PCMSK0 |= (1 << PCINT1);
  lastPin = PINB; // PINB is the state of every pin in the B set (PB#). it is an 8 bit int, but PINB is just using the binary part so 11111 is everything HIGH and 00000 is everything low
  
  EICRB |= (1<<ISC60); //Enabling int6 interrupt type
  EIMSK |= (1<<INT6); //Activing that interrupt
}

void loop() {
  for (int ii = 0; ii < 5; ii++) { // copy volatile to free channels
    freeChannels[ii] = channels[ii];
    Serial.print(freeChannels[ii]); // print em
    Serial.print("," );
  }
  Serial.println(); // new line
  delay(500); // wait a bit

}
// c macro which replaces the place where TEST_PIN is used with this code. It's faster and has less parameters!!!!
#define TEST_PIN(pin,diff) {                               \
  if (diff & channelBits[pin]) {                           \
    if (!(pin & channelBits[pin])) {                       \
      deltaT = currentT - risingEdge[pin];                 \
      if (900 < deltaT && deltaT < 2200) {                 \
        channels[pin] = deltaT;                            \
      }                                                    \
    } else risingEdge[pin] = currentT;                     \
  }                                                        \
}
ISR(PCINT0_vect) { // the interrupt for ALL of the B pins. This means that when any of them change state the interrupt is triggered
  uint8_t diff; // a diff between the pins and mask to see what's changed
  uint8_t pin; // the pins
  uint16_t currentT, deltaT; // timers
  static uint16_t risingEdge[5]; // stored rising edge
  pin = PINB; // set pin to what everything's state is right now
  diff = pin ^ lastPin; // do a XOR of pin and lastPin which means that if there is a difference that bit goes HIGH. ex: 1111 and 1101 would result in 0010
  currentT = micros(); // set start time to micros
  sei(); // reenable interrups because all timing sensitive stuff is done
  lastPin = pin; // set lastPin to pin for the next time the interrupt is called
  TEST_PIN(1, diff); // check every pin and calculate the uSeconds. Could be better
  TEST_PIN(2, diff);
  TEST_PIN(3, diff);
  TEST_PIN(4, diff);
}

ISR(INT6_vect) {
  uint16_t currentT, deltaT; // timers
  static uint16_t risingEdge; // stored rising edge
  currentT = micros(); // set start time to micros
  if (!(PINE & (1<<6))) { // pin e is the set that 7 is on. 7 is pin 6 for some reason so we shift it 6 over
    deltaT = currentT - risingEdge; // same code as testPin, but shorter because we don't need one of the if statements because of the external interrupt
    if (900 < deltaT && deltaT < 2200) {
      channels[0] = deltaT;
    }
  } else risingEdge = currentT;
}




// a method to convert 8 bit ints into a string that displays the int as binary
void toBin(uint8_t n) {
  char result[(sizeof(uint8_t) * 8) +1];
  uint8_t index = sizeof(uint8_t) * 8;
  result[index] = '\0';
  do result[--index] = '0' + (n&1);
  while(n >>= 1);
  Serial.println(String(result+index) + " ");
}
