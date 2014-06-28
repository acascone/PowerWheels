/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * Heavily modified by Steve Spence, http://arduinotronics.blogspot.com
 * Heavily modified by Hack Pittsburgh http://www.hackpittsburgh.org
 */

// Timeout for remote triggered fire in milliseconds
#define REMOTE_FIRE_TIMEOUT 5000
#define TELEMETRY_PERIOD_MS 100

#include <IRremote.h>
#include <Bounce.h>
#include <Servo.h>
#include <Alltrax.h>

typedef struct {
  float voltage;
  float current;
  int16_t temperature;
  int16_t throttle;
} msg_data_s;

// this is the header for an XBee TX64 Request
const uint8_t header[] = {0x7E,0x00,0x17,0x00,0x00,0x00,0x13,0xA2,0x00,0x40,0xA0,0xB1,0xD4,0x01};

Alltrax    controller;                  // object to talk with motor controller
msg_data_s motor_data;                  // structure to store motor controller state

unsigned long remoteFireStart;          // Remote Fire TMO
unsigned long motorDataTMO;             // Motor data telemetry TMO

int sillyStringLED =  4;             	// LED connected to digital pin
int smokeLED       =  5;               	// LED connected to digital pin
int caltropLED     =  6;             	// LED connected to digital pin
int waterGunLED    =  7;             	// LED connected to digital pin
int sillyStringBUT =  8;                // Button connected to digital pin
int smokeBUT       =  9;                // Button connected to digital pin
int caltropBUT     = 10;                // Button connected to digital pin
int waterGunBUT    = 11;                // Button connected to digital pin

int resetBUT       = 12;                // Reset Switch on pin 12
int TriggerBUT     = 13;                // Button connected to digital pin 13

int RECV_PIN       = 19;                // IR reciever

int waterGunRelay  = 32;                // 5V Relay connected to pin 32

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ Debouncing
Bounce caltropBounce     = Bounce(caltropBUT,     5);
Bounce waterGunBounce    = Bounce(waterGunBUT,    5);
Bounce smokeBounce       = Bounce(smokeBUT,       5);
Bounce sillyStringBounce = Bounce(sillyStringBUT, 5);

int caltropVal       = 0;
int caltropState     = 0;
int waterGunVal      = 0;
int waterGunState    = 0;
int smokeVal         = 0;
int smokeState       = 0;
int sillyStringVal   = 0;
int sillyStringState = 0;

IRrecv irrecv(RECV_PIN);
decode_results results;

int reset            = 0;
int TRIGGER          = 0;              // FIRE ZE MISSILES!!!!!!!!!!!!!!

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SERVOS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// SSservo (silly String) 
Servo SSservo;                // create servo object to control a servo 
Servo rightHinge;
Servo smokeBomb;
Servo leftHinge;
Servo caltropCover;

// Silly String Firing
void SSservoUp() {
  SSservo.write(100);
}
void SSservoDown() {
  SSservo.write(0);
}

// Silly String Hinge Control
void rightHingeUp() {
  rightHinge.write(0);
}
void rightHingeDown() {
  rightHinge.write(75);
}

// Smoke Fire Control
void airTubeOpen() {
  smokeBomb.write(180);
}
void airTubeCrimp() {
  smokeBomb.write(80);
}

// Smoke Hinge Control
void leftHingeUp() {
  leftHinge.write(170);
}
void leftHingeDown() {
  leftHinge.write(80);
}

// Caltrop Cover Control
void caltropCoverOpen() {
  caltropCover.write(180);
}
void caltropCoverClosed() {
  caltropCover.write(180);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ VOID SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  Serial.begin(9600);         // Debug serial port
  controller.begin(&Serial1); // start connection to motor controller on USART1
  irrecv.enableIRIn();        // Start the receiver
  
  pinMode(caltropLED,     OUTPUT);      // sets the digital pin as output
  pinMode(waterGunLED,    OUTPUT);      // sets the digital pin as output
  pinMode(smokeLED,       OUTPUT);      // sets the digital pin as output
  pinMode(sillyStringLED, OUTPUT);  	// sets the digital pin as output
  pinMode(waterGunRelay,  OUTPUT);      // sets the ditigal pin as output
  pinMode(caltropBUT,      INPUT);      // sets the digital pin as input
  pinMode(waterGunBUT,     INPUT);      // sets the digital pin as input
  pinMode(smokeBUT,        INPUT);      // sets the digital pin as input
  pinMode(sillyStringBUT,  INPUT);      // sets the digital pin as input
  pinMode(resetBUT,        INPUT);      // sets the digital pin as input
  pinMode(TriggerBUT,      INPUT);      // sets the digital pin as input
  
  SSservo.attach(22);                    // attaches the servo on pin 3 to the servo object
  SSservoUp();
  rightHinge.attach(24);                 // "      "      "      "    4      "    "
  rightHingeDown();
  smokeBomb.attach(26);
  airTubeCrimp();
  leftHinge.attach(28);
  leftHingeDown();
  caltropCover.attach(30);
  caltropCoverClosed();

  motorDataTMO = millis(); // set TMO to current time
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ OTHER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void CALTROPS() {
  if (digitalRead(caltropLED) == 0){
    digitalWrite(caltropLED, HIGH);   // sets the LED on
  } 
  else if (digitalRead(caltropLED) == 1) {
    digitalWrite(caltropLED, LOW);   // sets the LED off
  }
}

void WATERGUN() {
  if (digitalRead(waterGunLED) == 0){
    digitalWrite(waterGunLED, HIGH);   // sets the LED on
  } 
  else if (digitalRead(waterGunLED) == 1) {
    digitalWrite(waterGunLED, LOW);   // sets the LED off
  }
}

void SMOKE() {
  if (digitalRead(smokeLED) == 0){
    digitalWrite(smokeLED, HIGH);   // sets the LED on
    leftHingeUp();
  } 
  else if (digitalRead(smokeLED) == 1) {
    digitalWrite(smokeLED, LOW);   // sets the LED off
    leftHingeDown();
    airTubeCrimp();
  }
}

void SILLYSTRING() {
  if (digitalRead(sillyStringLED) == 0){
    digitalWrite(sillyStringLED, HIGH);   // sets the LED on
    rightHingeUp();
  } 
  else if (digitalRead(sillyStringLED) == 1) {
    digitalWrite(sillyStringLED, LOW);   // sets the LED off
    rightHingeDown();
    SSservoUp();
  }
}

void ledstop() {
  digitalWrite(waterGunLED,    LOW);   // sets the LED off
  digitalWrite(caltropLED,     LOW);   // sets the LED off
  digitalWrite(smokeLED,       LOW);   // sets the LED off
  digitalWrite(sillyStringLED, LOW);   // sets the LED off
  SSservoUp();
  rightHingeDown();
  leftHingeDown();
  airTubeCrimp();
  digitalWrite(waterGunRelay, LOW);
  caltropCoverClosed();
}

void mainFiringSequence() {
  if (digitalRead(sillyStringLED) == 1) {
    SSservoDown(); 
  }
  if (digitalRead(smokeLED) == 1) {
    airTubeOpen();
  }
  if (digitalRead(caltropLED) == 1) {
    caltropCoverOpen();
  }
  if (digitalRead(waterGunLED) == 1) {
    digitalWrite(waterGunRelay, HIGH);
  }
}

void stopShooting() {
  SSservoUp();
  digitalWrite(waterGunRelay, LOW);
  airTubeCrimp();
  caltropCoverClosed();
}



bool remoteFireFlag = false;

// If remoteFireFlag is set, this function checks if the remoteFireTimeout is reached
void checkRemoteTimeout() {
  if (remoteFireFlag && ((millis() - remoteFireStart) > REMOTE_FIRE_TIMEOUT)) remoteFireFlag = false;
}

void REMOTE_FIRE() {
  remoteFireStart = millis();
  remoteFireFlag = true;
}

enum {
JVC_ATT       = 0x0000F171L,
JVC_SOUND     = 0x0000F1B1L,
JVC_U         = 0x0000F129L,
JVC_D         = 0x0000F1A9L,
JVC_R         = 0x0000F1C9L,
JVC_F         = 0x0000F149L,
JVC_VOL_DOWN  = 0x0000F1A1L,
JVC_VOL_UP    = 0x0000F121L,
CASIO_CH_DOWN = 0x77C26DE8L,
CASIO_REW     = 0xA224542CL,
CASIO_FF      = 0x455951E0L,
CASIO_CH_UP   = 0x8B603BB8L,
CASIO_PLAY    = 0x5A78F08CL,
CASIO_STOP    = 0x58F71FB0L,
CASIO_TV      = 0xE2B0095FL,
CASIO_POWER   = 0xB8FC80C4L,
};

void sendMotorTelemetry() {
  // outgoing message buffer
  uint8_t buf[27];
  
  // prepare the outgoing message buffer
  memcpy(buf,header,sizeof(header));
  memcpy(buf+sizeof(header),&motor_data,sizeof(motor_data));
  
  // calculate the checksum
  uint8_t cs = 0;
  for (int i = 3; i < sizeof(buf)-1; i++) cs += buf[i];
  cs = 0xFF - cs;
  
  // set the checksum byte in the message
  buf[26] = cs;
  
  for(int i = 0; i < sizeof(buf); i++) Serial.write(buf[i]); // send data to XBee
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VOID LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  checkRemoteTimeout();
  TRIGGER = digitalRead(TriggerBUT);
  if (TRIGGER == HIGH || remoteFireFlag) {
    mainFiringSequence();
  } 
  else if (TRIGGER == LOW || !remoteFireFlag) {
    stopShooting();
  }

  reset = digitalRead(resetBUT);
  if (reset == HIGH) {
    ledstop();
  }

  if (millis() >= motorDataTMO) {        // check if it's time to send telemetry
    // Read motor controller values from alltrax
    motor_data.voltage     = controller.readVoltage();
    motor_data.temperature = controller.readTemp();
    motor_data.current     = controller.readCurrent();
    motor_data.throttle    = controller.readThrottle();
  
    sendMotorTelemetry();                // send data to xbee
    motorDataTMO += TELEMETRY_PERIOD_MS; //update timeout to next period
  }

  // call to the debouncing library/functions
  caltropBounce.update();
  waterGunBounce.update();
  smokeBounce.update();
  sillyStringBounce.update();

  // button inputs
  caltropVal     = caltropBounce.read();
  waterGunVal    = waterGunBounce.read();
  smokeVal       = smokeBounce.read();
  sillyStringVal = sillyStringBounce.read();

  if (caltropVal == true && caltropState == false) CALTROPS();
  caltropState = caltropVal;

  if (waterGunVal == true && waterGunState == false) WATERGUN();
  waterGunState = waterGunVal;

  if (smokeVal == true && smokeState == false) SMOKE();
  smokeState = smokeVal;

  if (sillyStringVal == true && sillyStringState == false) SILLYSTRING();
  sillyStringState = sillyStringVal;

  if (irrecv.decode(&results)) {
    long int decCode = results.value;
    Serial.println(decCode);
    switch (results.value) {

    case JVC_F:
    case CASIO_REW:
      CALTROPS();
      break;

    case JVC_U:
    case CASIO_CH_DOWN:
      WATERGUN();
      break;

    case JVC_D:
    case CASIO_CH_UP:
      SMOKE();
      break;

    case JVC_R:
    case CASIO_FF:
      SILLYSTRING();
      break;  

    case JVC_SOUND:
    case CASIO_STOP:
      ledstop(); // Reset
      break;

    case JVC_ATT:
    case CASIO_POWER:
      REMOTE_FIRE();
      break;

    default:
      Serial.println("Waiting ...");
    }

    irrecv.resume(); // Receive the next value
  }  
}

