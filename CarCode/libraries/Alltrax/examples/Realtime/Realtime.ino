/*
 PLEASE EXCERSIZE CAUTION IF YOUR VEHICLE IS CURRENTLY ON
 
  Alltrax Motor Controller Library - RealTime
 
 Demonstrates the use of the read functions included in this library in realtime.
 The Alltrax motor controller Library is compatible with Alltrax
 AXE/DCX motor controllers. It may work for other Alltrax series 
 motor controllers, however only the AXE series has been tested. 
 
 Based on work by Jennifer Holt ( http://jennwork.homelinux.net/drupal6/node/26 )
 and Alltrax ( http://www.mail-archive.com/listserv@electricmotorcycles.net/msg01779.html )
 
 This sketch demonstrates the use of the readThrottlePercent function in realtime
 This sketch will only work on Arduino Mega, Leonardo, Micro, Etc.
 The Uno, Duemilanove, and others do not have separate hardware and usb serial
 If you want to see this work on a Uno, use a LCD display or
 have the sketch interact with an LED
 
 
 The circuit:
 Note: a RS232 Adapter is necessary
 * Power and ground to RS232 shield/adapter
 * Arduino Rx to adapter Rx
 * Arduino Tx to adapter Tx
  Note: if you do not have a crossover or
  null modem cable, you may need to switch
  which pin goes to Rx and Tx.
  A crossover or null modem cable is simply one
  where the Tx and Rx lines are switch internally
  see http://upload.wikimedia.org/wikipedia/commons/5/57/D25_Null_Modem_Wiring.png
  for details.
 **
 
 Library published 15 May 2013
 by Derek Gutheil ( http://derekgutheil.com )
 
 This example code is in the public domain.

 http://derekgutheil.com/arduino-alltrax-axe-library/

 */
#include <Alltrax.h> //Import the motor controller Library

//This is how to properly set up a HardwareSerial version of Alltrax
//Note there is a begin() function for the HardwareSerial version, make sure that gets called
Alltrax controller;


void setup()
{
  controller.begin(&Serial1);
  //Alltrax setup complete
  
  Serial.begin(9600);//sets up the arduino to communicate with the computer
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo and Micro only
  }
}

void loop(){
  Serial.print("Throttle Percentage: ");
  Serial.println(controller.readThrottlePercent());
}
