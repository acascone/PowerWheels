/*
  Created by Derek K. Gutheil, March 22nd, 2013.
  Copyright (c) 2013 by Derek Gutheil <derekkg ~AT~ hotmail.com>

	Library for interfacing with an Alltrax AXE/DCX motor Controller. 
	It may work for other Alltrax series motor controllers, however
	only the AXE series has been tested. Based on work by Jennifer Holt
	(http://jennwork.homelinux.net/drupal6/node/26) and Alltrax
	(http://www.mail-archive.com/listserv@electricmotorcycles.net/msg01779.html)

    This file is part of the Arduino Alltrax Library.

    The Arduino Alltrax Library is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The Arduino Alltrax Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Arduino Alltrax Library.  If not, see
    <http://www.gnu.org/licenses/>. 
*/

#include "Arduino.h"
#include "Alltrax.h"


Alltrax::Alltrax() 
{
	_Serial = NULL;
}


void Alltrax::begin(HardwareSerial *serIn) {
   _Serial = serIn;
   _Serial->begin(9600);
}



int Alltrax::readTemp(){
  //This function returns the tempurature in celcius that the controller reports
  //Pre: none
  //Post: Celcius value of controller is returned as a float 
  
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x04,0x2C,0x00,0x00,0x00,0x75};
  returnArray = requestResponse(tempArray);

  if (returnArray[0] == 0){
    return -1;
  }
  //I think this will work. 
  //This should correctly convert the returned value to Celcius
  //This math might be slightly incorrect as well, one source says that there is 2.048 counts per degree celcius with a offset at 0 degrees celcius = 559 counts
  int temp = ((returnArray[4])*16*16)+(returnArray[3]);
  temp = temp - 559;
  int val = .5*temp;
  return val;
  //End return 
}

int Alltrax::readThrottle(){
  //This function returns the throttle position from 0 to 255
  //to get a percentage divide this number by 2.55
  //Pre: none
  //Post: throttle position is reported as an int between 0 and 255
  //      -1 if there was an error
  
  //byte array initialization
  byte* returnArray ;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x04,0x20,0x00,0x00,0x00,0x81};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  //The returned value by the controller will be a position on a scale of 0 through 255
  int throttlePos = returnArray[3];
  return throttlePos;
  //return 0;
}


float Alltrax::readThrottlePercent(){
  //This function returns the throttle position as a percentage from 0 to 100
  //Pre: none
  //Post: throttle position is reported as an float between 0 and 100
  //      -1 if there was an error
 int throttlePos = readThrottle();
 float returnVal = throttlePos/2.55;
 return returnVal;
}

float Alltrax::readVoltage(){
  //returns the battery Voltage
  //Pre: None
  //Post: Battery Voltage as a float
  //       -1 if there was an error
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x04,0x39,0x00,0x00,0x00,0x68};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  float val = ((returnArray[4])*16*16)+(returnArray[3]);
  float voltage = val * .1025;
  return voltage;
}

float Alltrax::readCurrent(){
  //returns the output Current
  //Pre: None
  //Post: returns the Output current as a float
  //      -1 if there was an error
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x04,0x60,0x00,0x00,0x00,0x41};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  float val = ((returnArray[4])*16*16)+(returnArray[3]);
  return val;
}

float Alltrax::readBatteryCurrent(){
  //returns the output battery Current
  // "Battery current may be calculated as a percentage of output current,
  // assuming continuous motor currrent.  Convert THROT_POS to decimal, then
  // Ibattery =  (THROT_POS / 256) * OUTPUT_CURR_H,L"
  //Pre: None
  //Post: returns the Output Battery current as a float
  //      -1 if there was an error
  
  float OutCur = readCurrent();
  int throttle = readThrottle();
  throttle = throttle/256;
  float batteryCurrent = throttle * OutCur;
  return batteryCurrent;
}

boolean Alltrax::resetController(){
  //resets the controller
  //Causes the controller to go to the START location and begin anew
  //Pre: None
  //Post: Controller is reset
  //byte array initialization
  //byte* returnArray;
  //send command to controller
  byte tempArray[7] = {0x5B,0x05,0x00,0x00,0x00,0x00,0xA0};
  return sendCommand(tempArray);//------Definitely resets controller, but also resets the arduino.....-------------------------------------------------------------------------------
}

byte Alltrax::readShutdown(){
  //checks to see if the controller is shutdown
  //returns 1 if there is an error, 0 if not
  //Pre: none
  //Post: 1 if error, -1 if transmition error, 0 if the controller is fine
 
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x04,0x3B,0x00,0x00,0x00,0x66};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  return returnArray[3];
}


int Alltrax::readMaxCurrent(){
  //returns the Max output current percentage
  //Pre: None
  //Post: returns the Output current as a int
  //      -1 if there was an error
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x02,0x04,0x00,0x00,0x00,0x9f};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  int val = (returnArray[4]);
  return val;  
}

boolean Alltrax::setMaxCurrent(int Percent){
  //sets the motor controller Max Current to the percent provided
  //Pre: None
  //Post: returns true if there were no errors,
  //      false if there was an error
  
  byte percentVal = Percent;
  //compute checksum
  byte checkSum = twoComplement(0x62 + percentVal);
  //create command to send
  byte tempArray[7] = {0x5B,0x03,0x04,0x00,percentVal,0x00,checkSum};
  //send command to controller
  return sendCommand(tempArray);
}


int Alltrax::readThrottleUpRate(){
  //returns the Throttle Up Rate
  //Pre: None
  //Post: returns the Throttle Up Rate as a int
  //      -1 if there was an error
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x02,0x09,0x00,0x00,0x00,0x9a};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  int val = (returnArray[4]);
  return val;
}

boolean Alltrax::setThrottleUpRate(int Up){
  //sets the motor controller Max Current to the int provided
  //Pre: None
  //Post: returns true if there were no errors,
  //      false if there was an error
  
  byte UpVal = Up;
  //compute checksum
  byte checkSum = twoComplement(0x67 + UpVal);
  //create command to send
  byte tempArray[7] = {0x5B,0x03,0x09,0x00,UpVal,0x00,checkSum};
  //send command to controller
  return sendCommand(tempArray);
}


int Alltrax::readTopSpeed(){
  //returns the Top Speed Percent
  //Pre: None
  //Post: returns the Top Speed Percent as a int
  //      -1 if there was an error
  
  //byte array initialization
  byte* returnArray;
  
  //send command to controller
  byte tempArray[7] = {0x5B,0x02,0x38,0x00,0x00,0x00,0x6b};
  returnArray = requestResponse(tempArray);
  if (returnArray[0] == 0){
    return -1;
  }
  int val = (returnArray[4]);
  return val;
}

boolean Alltrax::setTopSpeed(int MaxSpeed){
  //sets the motor controller Top Speed Percent to the provided percentage
  //Pre: None
  //Post: returns true if there were no errors,
  //      false if there was an error
  
  byte SpeedVal = MaxSpeed;
  //compute checksum
  byte checkSum = twoComplement(0x96 + SpeedVal);
  //create command to send
  byte tempArray[7] = {0x5B,0x03,0x38,0x00,SpeedVal,0x00,checkSum};
  //send command to controller
  return sendCommand(tempArray);
}


















boolean Alltrax::sendCommand(byte sendArray[]){
  //Sends the passed byte array over serial and returns a true
  //Pre: takes a byte array to send over serial
  //Post: returns true
  _Serial->write(sendArray, 7);
  return true;
}

byte* Alltrax::requestResponse(byte sendArray[]){
  //Sends the passed byte array over serial and returns its response, if there is one
  //Pre: takes a byte array to send over serial
  //Post: returns response, or 0 if there was a transmision error
  
  //return byte array initialization
  static byte returnArray[7];  
  _Serial->write(sendArray, 7);
  //make sure controller has time to respond
  if (!responseDelay()){
    return 0;
  }

  //read data from serial into returnArray
  int index = 0;
  while (_Serial->available()>0){
    returnArray[index]=_Serial->read();
    index += 1;
  }
  //end read from serial
  
  //check the checksum to make sure there were no transmition errors
  //I'm not sure if the byte addition works
  byte checksum = 0;
  for (int i = 0; i < 6; i++){
    checksum = checksum + returnArray[i];
  }
  checksum = twoComplement(checksum);
  if (checksum != returnArray[6]){
    return 0;
  }
  //end checksum check 
  return returnArray;  
}

boolean Alltrax::responseDelay(){
  // waits until there is 7 bytes in the serial buffer (IE the controller has responded).
  // returns true if 7 bytes showed up, false if it took more than 1000 milliseconds
  // pre: A command was sent to the controller
  // Post: there are at least 7 bytes in the serial buffer. False if there isn't
 int count = 0;
 while (_Serial->available() < 7){// 7 because we need to wait for all 7 response bytes to recieve
  delay(1);
  count++;
  if (count > 1000){
     return false; 
  }
 }
 return true; 
}

byte Alltrax::twoComplement(byte val){
 //takes a byte and returns the 2's complement of it, rolls over if it excedes 1 byte
 //pre: byte 
 //post: 2's complement
 byte returnByte;
 val = val ^ 0xFF;
 returnByte = val + 0x01;
 return returnByte;
}