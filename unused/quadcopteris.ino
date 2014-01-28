#define THRUST_IN_PIN
#define ROLL_IN_PIN
#define PITCH_IN_PIN
#define YAW_IN_PIN

//Values for the bit flag
#define THRUST_FLAG 1
#define ROLL_FLAG   2
#define PITCH_FLAG  4
#define YAW_FLAG    8

volatile uint8_t updateFlagsShared;

volatile uint16_t thrustInShared;
volatile uint16_t rollInShared;
volatile uint16_t pitchInShared;
volatile uint16_t yawInShared;

//These are used to measure timings in the interrupt functions.
uint32_t thrustStart;
uint32_t rollStart;
uint32_t pitchStart;
uint32_t yawStart;

void setup() {

	//Start listening for changes in thrust
	attachInterrupt(THRUST_IN_PIN, calcThrust, CHANGE); 
	attachInterrupt(ROLL_IN_PIN, calcRoll, CHANGE);
	attachInterrupt(PITCH_IN_PIN, calcPitch, CHANGE);
	attachInterrupt(YAW_IN_PIN, calcYaw, CHANGE); 
}

void loop() {
	//These variables hold local copies of input. They're static so that they persist in between calls to loop.
	static uint16_t thrustIn;
	static uint16_t rollIn;
	static uint16_t pitchIn;
	static uint16_t yawIn;
	
	static uint8_t updateFlags;
	
	if (updateFlagsShared) {
		
		byte sregRestore = SREG; //Disable interrupts while we grab the shared values
		cli();
		
		updateFlags = updateFlagsShared;
		
		if (updateFlags & THRUST_FLAG) { //Thrust has changed
			thrustIn = thrustInShared;
		}

		if (updateFlags & ROLL_FLAG) {   //Roll has changed
			rollIn = rollInShared;
		}

		if (updateFlags & PITCH_FLAG) {  //Pitch has changed
			pitchIn = pitchInShared;
		}

		if (updateFlags & YAW_FLAG) {    //Yaw has changed
			yawIn = yawInShared;
		}
		
		updateFlagsShared = 0; //Clear the flag, we've already checked all the updates.
		
		SREG = sregRestore; //Restore interrupts to whatever they were before. 
							//Past this point, we can no longer safely read the shared values.
	}
	
	//Do all of the signal processing and outputting to ESCs here
	
	updateFlags = 0; //We're done with the update flags.
}

void calcThrust() {
	//If the pin reads high, we are at the rising edge of the signal so we can measure the length.
	if (digitalRead(THRUST_IN_PIN) == HIGH) {
		thrustStart = micros();
	} else {
		//else it's a falling edge, lets subtract the current time from the start to see the duration.
		thrustInShared = (uint16_t)(micros() - thrustStart);
		//set the flag so that program knows throttle has changed
		updateFlagsShared |= THRUST_FLAG;
	}
}

void calcRoll() {
	if (digitalRead(ROLL_IN_PIN) == HIGH) {
		rollStart = micros();
	} else {
		rollInShared = (uint16_t)(micros() - rollStart);
		updateFlagsShared |= ROLL_FLAG;
	}
}

void calcPitch() {
	if (digitalRead(PITCH_IN_PIN) == HIGH) {
		pitchStart = micros();
	} else {
		pitchInShared = (uint16_t)(micros() - pitchStart);
		updateFlagsShared |= PITCH_FLAG;
	}
}

void calcYaw() {
	if (digitalRead(YAW_IN_PIN) == HIGH) {
		yawStart = micros();
	} else {
		yawInShared = (uint16_t)(micros() - yawStart);
		updateFlagsShared |= YAW_FLAG;
	}
}