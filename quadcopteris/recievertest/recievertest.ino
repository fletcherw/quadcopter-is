#include <avr/interrupt.h>
volatile int special = 0;
volatile uint16_t thrustShared;
volatile uint8_t updatedShared;
volatile uint32_t thrustStart;

uint16_t thrust;

#define PIN 2
#define OUTPIN 13
void setup(void) {
  Serial.begin(9600);
  pinMode(OUTPIN, OUTPUT);
  pinMode(PIN, INPUT);
  digitalWrite(PIN, HIGH);    // Enable pullup resistor (set pin to high and attach other side to ground)
  sei();                    // Enable global interrupts
  EIMSK |= (1 << INT0);     // Enable external interrupt INT0
  EICRA |= (1 << ISC00);    // Trigger INT0 on falling edge
}
                              //
void loop() {
  if (updatedShared) {
    thrust = thrustShared;
    updatedShared = 0;
  }
  Serial.println(thrust);
}
                              //
// Interrupt Service Routine attached to INT0 vector
ISR(INT0_vect) {
  if (digitalRead(PIN) == HIGH) {
    thrustStart = micros();
  } else {
    thrustShared = (uint16_t)(micros() - thrustStart);
    updatedShared = 1;
  }
  //digitalWrite(OUTPIN, !digitalRead(OUTPIN));    // Toggle LED on pin 13
}
