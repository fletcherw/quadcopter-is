#include <avr/interrupt.h>
uint8_t lastPin;
#define ALL_BITS (1<<1),(1<<2),(1<<3),(1<<4) // 00010, 00100, 01000, 10000

static uint8_t channelBits[4] = {ALL_BITS};
volatile uint16_t channels[4] = {1500, 1500, 1500, 1500}; // uSeconds for each channel
uint16_t freeChannels[4] = {1500, 1500, 1500, 1500}; // non volatile version
void setup() {
  //sei();
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
}

void loop() {
  for (int ii = 0; ii < 4; ii++) { // copy volatile to free channels
    freeChannels[ii] = channels[ii];
    Serial.print(freeChannels[0]); // print em
    Serial.print("," );
  }
  Serial.println(); // new line
  delay(500); // wait a bit

}

ISR(PCINT0_vect) { // the interrupt for ALL of the B pins. This means that when any of them change state the interrupt is triggered
  uint8_t diff; // a diff between the pins and mask to see what's changed
  uint8_t pin; // the pins
  uint16_t currentT, deltaT; // timers
  static uint16_t risingEdge[4]; // stored rising edge
  pin = PINB; // set pin to what everything's state is right now
  diff = pin ^ lastPin; // do a XOR of pin and lastPin which means that if there is a difference that bit goes HIGH. ex: 1111 and 1101 would result in 0010
  currentT = micros(); // set start time to micros
  sei(); // reenable interrups because all timing sensitive stuff is done
  lastPin = pin; // set lastPin to pin for the next time the interrupt is called
  testPin(0, diff, currentT, deltaT, risingEdge); // check every pin and calculate the uSeconds. Could be better
  testPin(1, diff, currentT, deltaT, risingEdge);
  testPin(2, diff, currentT, deltaT, risingEdge);
  testPin(3, diff, currentT, deltaT, risingEdge);
}

void testPin(uint8_t pin, uint8_t diff, uint16_t currentT, uint16_t deltaT, uint16_t risingEdge[]) { // calculates RC stuff
  if (diff & channelBits[pin]) { // checks if that pin is HIGH
    if (!(pin & channelBits[pin])) { // makes sure it wasn't HIGH last time (it changed)
      deltaT = currentT - risingEdge[pin]; // checks deltaT
      if (900 < deltaT && deltaT < 2200) { // if it is valid RC signal set it as an RC signal
        channels[pin] = deltaT;
      } else risingEdge[pin] = currentT; // else set the current time as the first edge of the wave
    }
  }
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
