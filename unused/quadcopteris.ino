#define THRUST_IN_PIN
#define ROLL_IN_PIN
#define PITCH_IN_PIN
#define YAW_IN_PIN

//Values for the bit flag
#define THRUST_FLAG 1
#define ROLL_FLAG   2
#define PITCH_FLAG  4
#define YAW_FLAG    8

#define THRUST_INDEX 0
#define ROLL_INDEX 1
#define PITCH_INDEX 2
#define YAW_INDEX 3


volatile uint8_t updateFlagsShared;

volatile uint16_t controlsShared[4];

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
	uint16_t controls[4];
	
	static uint8_t updateFlags;
	
	if (updateFlagsShared) {
		
		updateFlags = updateFlagsShared; //Safe to read once and not twice as it's only one byte.
		
		//No need to disable interrupts, we're using read twice and compare.
		
		if (updateFlags & THRUST_FLAG) { //Thrust has changed
			controls[THRUST_INDEX] = readTwice(&controlsShared[THRUST_INDEX]);
		}

		if (updateFlags & ROLL_FLAG) {   //Roll has changed
			controls[ROLL_INDEX] = readTwice(&controlsShared[ROLL_INDEX]);
		}

		if (updateFlags & PITCH_FLAG) {  //Pitch has changed
			controls[PITCH_INDEX] = readTwice(&controlsShared[PITCH_INDEX]);
		}

		if (updateFlags & YAW_FLAG) {    //Yaw has changed
			controls[YAW_INDEX] = readTwice(&controlsShared[YAW_INDEX]);
		}
		
		updateFlagsShared = 0; //Clear the flag, we've already checked all the updates.
	}
	
	//Do all of the signal processing and outputting to ESCs here
	
	updateFlags = 0; //We're done with the update flags.
}

uint16_t readTwice(uint16_t * var) {
	do {
		uint16_t oldVal = *var;
		uint16_t newVal = *var;
	} while (oldVal != newVal);
	
	return newVal;
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