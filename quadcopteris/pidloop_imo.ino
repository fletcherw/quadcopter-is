// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
 * ========================================================================= */

/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:

   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */








#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
uint16_t throttle = 1500;
Quaternion t;

uint8_t P[3];
uint8_t I[3];
uint8_t D[3];
uint8_t CurrentIntegral[3];
uint9_t OldTarget[3];

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void imuSetup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // wait for ready
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
    for (int ii = 0; ii<4; ii++) {
      motorValue[ii] = 500;
    }
    for (int ii = 0; ii<3; ii++) {
      P[ii] = 1000;
      I[ii] = 0;
      D[0] = 0;
    }
}



// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void imuLoop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
        // other program behavior stuff here
        // .
        // .
        // .
        // if you are really paranoid you can frequently test in between other
        // stuff to see if mpuInterrupt is true, and if so, "break;" from the
        // while() loop to immediately process the MPU data
        // .
        // .
        // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
        // display quaternion values in easy matrix form: w x y z
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        Serial.print("quat\t");
        Serial.print(q.w);
        Serial.print("\t");
        Serial.print(q.x);
        Serial.print("\t");
        Serial.print(q.y);
        Serial.print("\t");
        Serial.print(q.z);
        Serial.print("\t");
        Serial.print("\t");
        Serial.print(motorValue[0]);
        Serial.print("\t");
        Serial.print(motorValue[1]);
        Serial.print("\t");
        Serial.print(motorValue[2]);
        Serial.print("\t");
        Serial.println(motorValue[3]);

        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
    }
    
    //PID LOOP PROPORTIONAL TERMS HERE -------------------------
      
      uint8_t xErr = q.x - t.x;
      uint8_t yErr = q.y - t.y;
      uint8_t zErr = q.z - t.z;
    
      //roll
      motorValue[0] = throttle + xErr * P[0];
      motorValue[1] = throttle + xErr * -P[0];
      motorValue[2] = throttle + xErr * -P[0];
      motorValue[3] = throttle + xErr * P[0];
      
      //pitch
      motorValue[0] += yErr * -P[1];
      motorValue[1] += yErr * -P[1];
      motorValue[2] += yErr * P[1];
      motorValue[3] += yErr * P[1];
      
      //yaw
      motorValue[0] += zErr * -P[2];
      motorValue[1] += zErr * P[2];
      motorValue[2] += zErr * -P[2];
      motorValue[3] += zErr * P[2];
    //PID LOOP INTEGRAL TERMS HERE -----------------------------
      //reset the integrals if targets have changed, otherwise add to them
      if(OldTarget[0] == t.x) {
        CurrentIntegral[0] += xErr;
      }
      else {
        CurrentIntegral[0] = 0;
      }
      if(OldTarget[1] == t.y) {
        CurrentIntegral[1] += xErr;
      }
      else {
        CurrentIntegral[1] = 0;
      }
      if(OldTarget[2] == t.z) {
        CurrentIntegral[2] += xErr;
      }
      else {
        CurrentIntegral[2] = 0;
      }
      
      //roll
      //motorValue[0] += xErr * I[0];
      //motorValue[1] += xErr * -I[0];
      //motorValue[2] += xErr * -I[0];
      //motorValue[3] += xErr * I[0];
      
      
      //set oldtargets
       OldTarget[0] = t.x;
       OldTarget[1] = t.y;
       OldTarget[2] = t.z;
    //----------------------------------------------------------
          
      
    //if the values are higher or lower than they should be (unlikely), set them back to a reasonable range
    for (int ii = 0; ii < 4; ii++) {
      if (motorValue[ii] < 1000)
        motorValue[ii] = 1000;
      if (motorValue[ii] > 2000)
        motorValue[ii] = 2000;
    }
}
