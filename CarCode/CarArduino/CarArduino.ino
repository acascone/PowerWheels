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

#include <IRremote.h>
#include <Bounce.h>
#include <Servo.h>

int sillyStringLED = 4;             	// LED connected to digital pin
int smokeLED = 5;               	// LED connected to digital pin
int caltropLED = 6;             	// LED connected to digital pin
int waterGunLED = 7;             	// LED connected to digital pin
int sillyStringBUT = 8;                 // Button connected to digital pin
int smokeBUT = 9;                       // Button connected to digital pin
int caltropBUT = 10;                    // Button connected to digital pin
int waterGunBUT = 11;                   // Button connected to digital pin

int resetBUT = 12;                      // Reset Switch on pin 12
int TriggerBUT = 13;                    // Button connected to digital pin 13

int RECV_PIN = 19;                      // IR reciever

int waterGunRelay = 32;                 // 5V Relay connected to pin 32

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ Debouncing
Bounce caltropBounce = Bounce( caltropBUT, 5);
Bounce waterGunBounce = Bounce( waterGunBUT, 5);
Bounce smokeBounce = Bounce( smokeBUT, 5);
Bounce sillyStringBounce = Bounce( sillyStringBUT, 5);
int caltropVal = 0;
int caltropState = 0;
int waterGunVal = 0;
int waterGunState = 0;
int smokeVal = 0;
int smokeState = 0;
int sillyStringVal = 0;
int sillyStringState = 0;

IRrecv irrecv(RECV_PIN);
decode_results results;

int reset = 0;
int TRIGGER = 0;                        // FIRE ZE MISSILES!!!!!!!!!!!!!!

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SERVOS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// SSservo (silly String) 
Servo SSservo;                // create servo object to control a servo 
Servo rightHinge;
Servo smokeBomb;
Servo leftHinge;
Servo caltropCover;

// Silly String Firing
void SSservoUp() {
  SSservo.write(80);
}
void SSservoDown() {
  SSservo.write(80);
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
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(caltropLED, OUTPUT);  	// sets the digital pin as output
  pinMode(waterGunLED, OUTPUT);  	// sets the digital pin as output
  pinMode(smokeLED, OUTPUT);  	        // sets the digital pin as output
  pinMode(sillyStringLED, OUTPUT);  	// sets the digital pin as output
  pinMode(waterGunRelay, OUTPUT);       // sets the ditigal pin as output
  pinMode(caltropBUT, INPUT);           // sets the digital pin as input
  pinMode(waterGunBUT, INPUT);          // sets the digital pin as input
  pinMode(smokeBUT, INPUT);             // sets the digital pin as input
  pinMode(sillyStringBUT, INPUT);       // sets the digital pin as input
  pinMode(resetBUT, INPUT);             // sets the digital pin as input
  pinMode(TriggerBUT, INPUT);           // sets the digital pin as input
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
  
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ OTHER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void CALTROPS() {
  if (digitalRead(caltropLED) == 0){
  digitalWrite(caltropLED, HIGH);   // sets the LED on
  } else if (digitalRead(caltropLED) == 1) {
  digitalWrite(caltropLED, LOW);   // sets the LED off
  }
}

void WATERGUN() {
  if (digitalRead(waterGunLED) == 0){
  digitalWrite(waterGunLED, HIGH);   // sets the LED on
  } else if (digitalRead(waterGunLED) == 1) {
  digitalWrite(waterGunLED, LOW);   // sets the LED off
  }
}

void SMOKE() {
  if (digitalRead(smokeLED) == 0){
  digitalWrite(smokeLED, HIGH);   // sets the LED on
  leftHingeUp();
  } else if (digitalRead(smokeLED) == 1) {
  digitalWrite(smokeLED, LOW);   // sets the LED off
  leftHingeDown();
  airTubeCrimp();
  }
}

void SILLYSTRING() {
  if (digitalRead(sillyStringLED) == 0){
  digitalWrite(sillyStringLED, HIGH);   // sets the LED on
  rightHingeUp();
  } else if (digitalRead(sillyStringLED) == 1) {
  digitalWrite(sillyStringLED, LOW);   // sets the LED off
  rightHingeDown();
  SSservoUp();
  }
}

void ledstop() {
  digitalWrite(waterGunLED, LOW);   // sets the LED off
  digitalWrite(caltropLED, LOW);   // sets the LED off
  digitalWrite(smokeLED, LOW);   // sets the LED off
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


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VOID LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  
  TRIGGER = digitalRead(TriggerBUT);
  if (TRIGGER == HIGH) {
    mainFiringSequence();
  } else if (TRIGGER == LOW) {
    stopShooting();
  }
    
  
  reset = digitalRead(resetBUT);
  if (reset == HIGH) {
    ledstop();
    
  }
    
  // call to the debouncing library/functions
  caltropBounce.update();
  waterGunBounce.update();
  smokeBounce.update();
  sillyStringBounce.update();
  
  // button inputs
  caltropVal = caltropBounce.read();
  waterGunVal = waterGunBounce.read();
  smokeVal = smokeBounce.read();
  sillyStringVal = sillyStringBounce.read();
  
  if (caltropVal == true && caltropState == false) {
    CALTROPS();
  }
  caltropState = caltropVal;
  
  if (waterGunVal == true && waterGunState == false) {
    WATERGUN();
  }
  waterGunState = waterGunVal;
  
  if (smokeVal == true && smokeState == false) {
    SMOKE();
  }
  smokeState = smokeVal;
  
  if (sillyStringVal == true && sillyStringState == false) {
    SILLYSTRING();
  }
  sillyStringState = sillyStringVal;
  
  
  
  if (irrecv.decode(&results)) {
    
	long int decCode = results.value;
	Serial.println(decCode);
	switch (results.value) {
        
        // For FOB
  	case 61737:
          CALTROPS();
        break;
        
  	case 61897:
          WATERGUN();
    	break;
  	
        case 61769:
          SMOKE();
    	break;
  	
        case 61865:
          SILLYSTRING();
    	break;  
  	
        case 61809:
          ledstop();
    	break;

        // For Watch      	
  	case 688268197:
          CALTROPS();
        break;
        
  	case 694752261:
          WATERGUN();
    	break;
  	
        case -1678958473:
          SMOKE();
    	break;
  	
        case -1139104511:
          SILLYSTRING();
    	break;  
  	
        case -786483649:
          ledstop();
    	break; 
  	
        default:
    	Serial.println("Waiting ...");
	}

	irrecv.resume(); // Receive the next value
  }  
}
