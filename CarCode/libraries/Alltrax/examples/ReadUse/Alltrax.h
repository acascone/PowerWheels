/*
  Created by Derek K. Gutheil, March 22nd, 2013.
  Copyright (c) 2013 by Derek Gutheil <derekkg ~AT~ hotmail.com>

	Library for interfacing with an Alltrax AXE/DCX motor Controller.
	NOTE: an RS232 converter is necessary!
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
//#include <SoftwareSerial.h>
#ifndef Alltrax_h
#define Alltrax_h


class Alltrax
{
public:

	/** Constructor for Alltrax class
	* @Param 
	* @Return 
	*/
	Alltrax();

	/** Begin method
	* No need to call Serial.Begin, just this
	*
	* Takes a HardwareSerial object.
	* This depends on your Arduino Board.
	* Arduino Mega (ATmega2560) and Arduino Due (SAM3X8E) have 4 that can be used: Serial, Serial1, Serial2, Serial3
	* Note: Due lines are 3.3v not 5v lines, you may need a different RS232 Converter
	* Arduino Micro and Leonardo (ATmega32u4) need to use Serial1 to use the hardware serial. Serial is usb serial for those
	* Every other Arduino (Uno, Duemilanove, all ATmega328 boards): Serial
	* 
	* @Param HardwareSerial object
	* @Return none
	*/
	void begin(HardwareSerial *serIn);

	/** Returns the tempurature in celcius that the controller reports
    * @Param none
    * @return Celcius value of controller is returned as a float 
	*/
	int readTemp();

	/** Returns the throttle position from 0 to 255
	* to get a percentage divide this number by 2.55 (or use readThrottlePercent)
	* @Param none
	* @Return throttle position is reported as an int between 0 and 255
	*			-1 if there was an error
	*/
	int readThrottle();

	/** Returns the throttle position as a percentage from 0 to 100
	* @Param none
	* @Return throttle position is reported as an float between 0 and 100
	*			-1 if there was an error
	*/
	float readThrottlePercent();
	
	/** Returns the battery Voltage
	* @Param none
	* @Return Battery Voltage as a float
	*			-1 if there was an error
	*/
	float readVoltage();

	/** Returns the Output Current
	* @Param none
	* @Return Output Current as a float
	*			-1 if there was an error
	*/
	float readCurrent();

	/** Returns the Battery Output Current
	* Logic:
	*	"Battery current may be calculated as a percentage of output current,
	*	assuming continuous motor currrent.  Convert THROT_POS to decimal, then
	*	Ibattery =  (THROT_POS / 256) * OUTPUT_CURR_H,L"
	*
	* @Param none
	* @Return Battery Output Current as a float
	*			-1 if there was an error
	*/
	float readBatteryCurrent();

	/** Resets the controller
	* Causes the controller to go to the START location and begin anew
	* @Param None
	* @Retun Boolean (always true for now)
	*		Controller is reset
	*/
	boolean resetController();

	/** checks to see if the controller is shutdown
	* returns 1 if there is an error, -1 if there was a transmition error, 0 if not
	* @Param none
	* @Return 1 if error, -1 if transmition error, 0 if the controller is fine
	*/
	byte readShutdown();

	/** returns the Max output current percentage
	* @Param None
	* @Return returns the Output current percentage as a int
			  -1 if there was an error
	*/
	int readMaxCurrent();
	
	/** sets the motor controller Max Current to the percent provided
	* NOTE: Setters do NOT check to see if their changes were made correctly.
	*		It is recomended that you use a read function to check that the value was set correctly
	* @Param (int) Max Current Percent
	* @Return true (always true for now)
	*/
	boolean setMaxCurrent(int Percent);

	/** returns the Throttle Up Rate
	* @Param None
	* @Return returns the Throttle Up Rate as a int
			  -1 if there was an error
	*/
	int readThrottleUpRate();

	/** sets the motor controller Throttle Up Rate to the int provided
	* NOTE: Setters do NOT check to see if their changes were made correctly.
	*		It is recomended that you use a read function to check that the value was set correctly
	* @Param (int) Throttle Up Rate
	* @Return true (always true for now)
	*/
	boolean setThrottleUpRate(int Up);

	/** returns the Top Speed Percent
	* @Param None
	* @Return returns the Top Speed Percent as a int
			  -1 if there was an error
	*/
	int readTopSpeed();

	/** sets the motor controller Top Speed Percent to the Percentage provided
	* NOTE: Setters do NOT check to see if their changes were made correctly.
	*		It is recomended that you use a read function to check that the value was set correctly
	* @Param (int) Top Speed Percent
	* @Return true (always true for now)
	*/
	boolean setTopSpeed(int MaxSpeed);
private:
	boolean sendCommand(byte sendArray[]);
	byte* requestResponse(byte sendArray[]);
	boolean responseDelay();
	byte twoComplement(byte val);
	//SoftwareSerial *_Serial;
	HardwareSerial *_Serial;
};
#endif