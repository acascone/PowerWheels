/*
*
 *  20140511  GEM  PIT_TELEMETRY_REV001
 In response to PIT_TELEMETRY_REV001 transmitting alive msg,
 CAR arduino responds with cm1PB_Status.
 Add logic to PIT_TELEMETRY_REV001 to use cm1PB_Status
 *
 *Telemetry Program...
 *Gather telemetry from Power Wheels CAR
 *Telemetric data consists of 3 analog temperatures
 *& up to 6 digital signals (ARM1 thru 4, FIRE, PitIn).
 *Processing program will create Remote AT command records
 *& communicate them to PIT XBee module which will broadcast
 *them to the CAR XBee, wait for its response and transmit
 *them to Processing over the USB connector.  Processing will
 *parse data, take action based on parsed data & display on CRT.
 *Doing this potentially frees up the UART DIN/DOUT pins on XBee
 *& saves an Arduino.
 *THE CIRCUIT:
 
 CAR:
 XMTR
 COM12
 Serial#13a200409c9da6
 
 
 CAR:
 RCVR
 COM13
 Serial#13a20040a0b1d4
 
 Thermistor1 XBee Pin20
 Thermistor1 XBee Pin19
 Thermistor1 XBee Pin18
 
 Arm1 XBee Pin17
 Arm2 XBee Pin16
 Arm3 XBee Pin12
 Arm4 XBee Pin11
 FIRE XBee Pin15or9
 
 *
 *20140510  GEM  PIT_Telemetry_REV000
 *Start with Telemetry_REV016 
 *
 *20140511  GEM  PIT_Telemetry_REV001
 *Add ALIVE capability 
 *
 *20140512  GEM  PIT_Telemetry_REV002
 *Add send_TxRequest16()
 * 
 *20140513  GEM  PIT_Telemetry_REV003
 *Fix initializing of graphic switches to field (CAR) on startup of PIT processing program 
 *
 *20140513  GEM  PIT_Telemetry_REV004
 *Add capability to read I/O Samples using RxResponseIoSample 
 *20140514:  PIT_Telemetry_REV004 sketch has been tested with CAR_REV003.
 *           Have to fix toggling of switches...only cm1 sw reacts (even when cm2 sw is toggled 
 *20140513  GEM  PIT_Telemetry_REV005
 *Add brute force logic to correctly communicate to the CAR which switch is being toggled 
 *
 *20140520  GEM  PIT_Telemetry_REV006
 *see Black Notebook thoughts from 20140521
 *
 *20140525  GEM  PIT_Telemetry_REV008
 *change matchSwitchesWithData to light the lights correctly on all the switches.
 *
 *20140528  GEM  PIT_Telemetry_REV009
 *Read and display analog values (throttle position, voltage, current) from CAR.
 *
 *20140530  GEM  PIT_Telemetry_REV010
 *Lots of PB latency issues...cannot consistently toggle PB
 *
 *20140602  GEM  PIT_Telemetry_REV011
 *Rewrite transmit/receive functions using knowledge gained from ReceiveData_REV0_0.pde
 *
 *20140603  GEM  PIT_REV0_0
 *Trying to work thru latency/missing transmissions issues
 *
 *20140603  GEM  PIT_REV0_1
 *Change toggle request logic so that PIT sees transmission from CAR to set state of s/w cmLED.
 *Add capability to write output file.
 *
 *20140611  GEM  PIT_REV1_0
 *Clean up code
 *
 *20140615  GEM  PIT_REV1_1
 *Clean up code
 *
 *20140616  GEM  PIT_REV2_0
 *Add capability to interpret motor temperature data transmitted by CAR from its motor temperature thermistor, and display it on graphic.
 *This data is in ADC counts as transmitted by CAR...car needs to convert to temperature value like degC.
 *TODO...add conversion of temperature data from degC to degF.
 *
 *20140617  GEM  PIT_REV2_1
 *Cleanup display of data on graphic
 *(Add capability to interpret motor temperature data transmitted by CAR from its motor temperature thermistor, and display it on graphic.
 *This data is in ADC counts as transmitted by CAR...car needs to convert to temperature value like degC.
 *TODO...add conversion of temperature data from degC to degF.)
 *
 *20140618  GEM  PIT_REV2_2
 *Add ALIVE ACK light
 *Changed CAR_REV7_1 to transmit motor temperature in degF.
 *
 *20140618  GEM  PIT_REV2_3
 *Add 2 sec delay after transmitting ALIVE msg so that subsequent transmissions (such as requests to toggle cmLEDs) do not collide
 *
 *20140619  GEM  PIT_REV2_4
 *Changed try/catch in checkForReceivedPacket() so that logic is only executed if packet is received.
 *TODO explore combining logic from checkForReceivedPacket() and processReceivedData().
 *
 *20140625  GEM  PIT_REV3_0
 *Combine checkForReceivedPacket() function and processReceivedData().
 * 
 */

// Need G4P library (for thermometers???)
//import g4p_controls.*;

import java.util.*;
import java.text.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.lang.Object;

import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.PacketListener;
import com.rapplogic.xbee.api.wpan.TxStatusResponse;

// used for communication via xbee api
import processing.serial.*; 
String version = "REV1_0";

// create and initialize a new xbee object
XBee xbee = new XBee();
int[] data;
int msb = 0;
int lsb = 0;
int options = 0;
XBeeResponse response;
TxStatusResponse tx_st_response;
IoSample[] ioData;
RxResponse16 rxResponse;
TxRequest16 request;
TxRequest16 aliveMsg;
int responseType = 0;  //variable indicating type of XBee data frame received
int motorControlTemperatureValue = 0;    //variable contains motor control analog temp value
int tyme = millis();
int numCounterMeasures = 4;  //number of counter measures on CAR
boolean[] toggleReq = new boolean[4];  //boolean array used to keep track of which counter measure switch is being toggled

final boolean LOG_SERIAL = false;

int temporaryTotal = 0;    //variable to store analog totals to display on indicators

int error=0;  //XBee error indicator
// make an array list of indicator objects for display of thermometers
//ArrayList<Indicator> indicators;


// make an array list of light objects for display
ArrayList<Light> lights;  //an arrayList for all the lights

// make an array list of switch objects for display
ArrayList<Switch> switches;  //an arrayList for all the switches
//ArrayList switches = new ArrayList();
//ArrayList nodes = new ArrayList();

// create a font for display
PFont font;
//float lastNodeDiscovery;

Serial myPort;  // The serial port
String myString = null;
int cr = 13;  //10;    // Carriage return in ASCII

long elapsedTime = 0;

//String sending;

//int[] serialInArray = new int[3];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive
boolean firstContact = false;        // Whether we've heard from the microcontroller
//int numSwitches = 7;  //Number of switches displayed on screen
int switchCounter = -1;  //keep track of which switch is toggled

int Tx64_request_cmd[] = new int[20];

//******************Start Indicator Variables***************
//define labels for indicators
final String M = "M";  //motor1 temp
final String C = "C";  //controller temp
final String N = "N";  //motor2 temp
final String T = "T";  //throttle pos
final String V = "V";  //voltage 
final String I = "I";  //current (amps)

String[] list = new String[2];


PrintWriter output;
String volts = null;
String amps = null;
String mTemp = null;  //motor1 temp
String cTemp = null;  //controller temp
String nTemp = null;  //motor2 temp
String throtPos = null;  //power wheels car throttle position

String param = null;  //generic variable to hold list[1] after trim

int i = 0;

// make an array list of indicator objects for display of thermometers
ArrayList<Indicator> indicators;

// make an array list of thermometer objects for display
//ArrayList thermometers = new ArrayList();

int displayValue = 0;  //variable passed to getXCTUData

PImage img;

int paramInt = 0;  //list[0]

//********************End Indicator Variables***************

//********************Start Common Variables******************
int counter = 0;
//********************End Common Variables********************

//Define number of modules here as a constant
int numb_of_modules = 1;

//Define number of bytes in arrays as constants
int dataISlength = 40;

//Create an array for the response from each End-device module
int [][]dataIS = new int[numb_of_modules][dataISlength];
//int dataIS[][];

//Define ADC reference-voltage input, change as needed.
float Vref = 3.3;

//Define digital-output pin for optional "error" LED
int    LedPin = 13;

//Define additional temporary variables
int             XBee_numb;
int    bytecount;
float           analog_voltage;

int receiveError = 0;  //if =1 discard XBee data
int controller_temperature_error = 0;  //if = 1 then XBee reveived a bogus reading from motor controller
int motor_temperature_error = 0;  //if = 1 then XBee reveived a bogus reading from motor controller
int checksum_err = 0;

boolean any_switch_armed = false;

int pLoad = 0;
int aliveTime = 0;    //variable to store time of successful response from Alive msg.

void checkForReceivedPacket()
{
  try
  {
    response = xbee.getResponse(10000);
    // we got a response!
    if (LOG_SERIAL)
    {
      println("in checkForReceivedPacket()...we got a response!");
      println("in checkForReceivedPacket()...response.getApiId(): " + response.getApiId());
    }

    if (response.getApiId() == ApiId.RX_16_RESPONSE)
    {
      responseType = 81;    //set responseType for use in processReceivedPacket() function
      // since this API ID is RX_16_RESPONSE, we know to cast to 16
      rxResponse = (RxResponse16) response;
      if (LOG_SERIAL)
      {
        println("remote address: " + rxResponse.getRemoteAddress());
      }
      data = rxResponse.getData();
      if (LOG_SERIAL)
      {
        print("data...");
        for (int i = 0; i < data.length; i++)
        {
          //      print("data[" + i + "]" + " " + char(data[i]));
          print(char(data[i]));
        }
        println();
      }
      pLoad = 0;
      for (int i = 1; i < data.length; i++)
      {
        pLoad += pow(10, (4-i)) * (data[i]-48);
        if (LOG_SERIAL)
        {
          println("i/data[i]/char(data[i])/pLoad: " + i + " / " + data[i] + " / " + char(data[i]) + " / " + pLoad);
        }
      }
      if (LOG_SERIAL)
      {
        println("pLoad: " + pLoad);
      }

      if (LOG_SERIAL)
      {
        for (int i = 0; i < data.length; i++)
        {
          println("DECIMAL data[" + i + "]" + " " + (data[i]));
          println("HEX data[" + i + "]" + " " + hex(data[i]));
        }
      }
{
    boolean st8 = false;  //Local variable representing state of counter measure LED

    if (data[0] == 'C' & data[1] == 'M')
    {
      //We know that we are responding to a CMxHI or CMxLO command from CAR
      if (data[3] == 'L')
      {
        st8 = false;
      }
      else
      {
        st8 = true;
      } 
      ((Switch) switches.get(data[2] - 49)).state = st8;
      //      ((Switch) switches.get((char)data[2] - 0)).state = st8;
      int swNum = data[2] - 49;
      if (LOG_SERIAL)
      {
        println("saw " + (char)data[0] + (char)data[1] + (char)data[2] + (char)data[3] + (char)data[4]);
        println("setting counter measure sw" + swNum + " " + (char)data[3] + (char)data[4]);
      }
    }
    if (data[0] == 'T')
    {
      ((Indicator) indicators.get(1)).temp = pLoad;
    }
    else if (data[0] == 'P')
    {
      ((Indicator) indicators.get(2)).temp = pLoad;
    }
    else if (data[0] == 'V')
    {
      ((Indicator) indicators.get(3)).temp = pLoad;
    }
    else if (data[0] == 'I')
    {
      ((Indicator) indicators.get(4)).temp = pLoad;
    }

    else if (data[0] == 'M')
    {
      ((Indicator) indicators.get(0)).temp = pLoad;
    }
    else
    {
      //we got an unexpected RX_16_RESPONSE
    }
  }
  }
    if (response.getApiId() == ApiId.RX_16_IO_RESPONSE)
    {
      responseType = 83;    //set responseType for use in processReceivedPacket() function
      if (LOG_SERIAL)
      {
        println("ApiId.RX_16_IO_RESPONSE recognized");
      }

      // since this API ID is RX_16_IO_RESPONSE, we know to cast to RxResponseIoSample
      RxResponseIoSample ioSample = (RxResponseIoSample)response;
      if (LOG_SERIAL)
      {
        println("Got an RX_16_IO_RESPONSE...RxResponseIoSample ioSample has been made");
      }

      for (IoSample sample: ioSample.getSamples())
      {          
        if (ioSample.containsAnalog())
        {
          System.out.println("Analog pin 20 10-bit reading is " + sample.getAnalog0());
          System.out.println("Analog pin 19 10-bit reading is " + sample.getAnalog1());
          motorControlTemperatureValue = sample.getAnalog1();
          System.out.println("Analog pin 18 10-bit reading is " + sample.getAnalog2());
        } 
        else
        {
          // we know it's change detect since analog was not sent
          //        log.info("Received change detect for Digital pin 12: " + (sample.isD7On() ? "on" : "off"));
        }
      }

      if (LOG_SERIAL)
      {
        println("Analog1 value: " + motorControlTemperatureValue);
      }
      ((Indicator) indicators.get(0)).temp = (motorControlTemperatureValue);
      analog_voltage = motorControlTemperatureValue * Vref / 1024;
      if (LOG_SERIAL)
      {
        print("A1 Voltage:  ");
        print(analog_voltage);
        print("\n\r");
      }
    }
  } 
  catch (XBeeTimeoutException e)
  {
    //    if (LOG_SERIAL)
    //    {
    println("in checkForReceivedPacket()...we timed out without a response");
    println("in checkForReceivedPacket()...got XBeeTimeoutException: " + e);
    //    }
    // we timed out without a response
  }
  catch (XBeeException e)
  {
    //    if (LOG_SERIAL)
    //    {
    println("in checkForReceivedPacket()...got XBeeException: " + e);
    //    }
  }
}

void setup()
{
  size(1200, 1000); // screen size
  smooth(); // anti-aliasing for graphic display

  // You’ll need to generate a font before you can run this sketch.
  // Click the Tools menu and choose Create Font. Click Sans Serif,
  // choose a size of 10, and click OK.
  //  font =  loadFont("SansSerif-10.vlw");
  font =  loadFont("Arial-Black-48.vlw");
  textFont(font);

  // The log4j.properties file is required by the xbee api library, and 
  // needs to be in your data folder. You can find this file in the xbee
  // api library you downloaded earlier
  PropertyConfigurator.configure(dataPath("")+"log4j.properties");

  if (LOG_SERIAL)
  {
    println("Trying to open XBee Serial Port");
  }
  try
  {
    xbee.open("COM12", 9600);
  }
  catch (XBeeException e)
  {
    if (LOG_SERIAL)
    {
      println("XBee Serial Port NOT Open");
    }
  }
  // Create a new file in the sketch directory to store the data coming from the Power Wheels CAR
  DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd@HH_mm_ss");
  Date d = new Date();
  String fileName = formatter.format(d);
  //println(fileName);
  String extension = ".txt";
  fileName = fileName + extension;
  if (LOG_SERIAL)
  {
    println(fileName);
  }
  output = createWriter(fileName);

  // create a switch object for each node that doesn't have one yet
  switches = new ArrayList<Switch>();
  switches.add(new Switch(0, "Counter Measure 1"));
  switches.add(new Switch(1, "Counter Measure 2"));
  switches.add(new Switch(2, "Counter Measure 3"));
  switches.add(new Switch(3, "Counter Measure 4"));
  indicators = new ArrayList<Indicator>();
  if (LOG_SERIAL)
  {
    println("calling Thermometer");
  }
  indicators.add(new Thermometer(M, 35, 450, 0 * 75 + 40, 20, "Motor Temperature", "˚F", 0, 300));
  //  indicators.add(new Thermometer(N, 35, 450, 1 * 75 + 40, 20, "Motor2 Temperature", "˚F", 0, 1023));
  indicators.add(new Thermometer(C, 35, 450, 2 * 75 + 40, 20, "Controller Temperature", "˚F", 0, 100));
  indicators.add(new AnalogMeter(T, 450, 40, 375, 140, "Throttle Position", "%", 0, 100));
  indicators.add(new VoltMeter(V, 450, 40, 375, 240, "Voltage", "Volts", 0, 45));
  indicators.add(new AnalogMeter(I, 450, 40, 375, 340, "Current", "Amps", 0, 300));
  lights = new ArrayList<Light>();
  lights.add(new Light("Alive ACK", 100, 100, 3*140 + 40, 650, 0));

  img = loadImage("wide_HackPGH_Logo.png");

  //initialize toggleReq array
  for (int i = 0; i < numCounterMeasures; i++)
  {
    println("initializing toggleReq[" + i + "]");
    toggleReq[i] = false;
  }
}  //END setup()

// draw loop executes continuously
//String indType = null;

void formALIVE_Msg()
{
  // create a unicast packet to be delivered to remote radio with 16-bit address: 0002, with payload "ALIVE"
  aliveMsg = new TxRequest16(new XBeeAddress16(0x00, 0x02), new int[] {
    'A', 'L', 'I', 'V', 'E'
  }
  );
}

void draw()
{
  background(255); // draw a white background

  imageMode(CORNERS);
  image(img, width-200, 0, width, 200);

  //send ALIVE msg to notify CAR that PIT Telemetry is up
  // start transmitting after a startup delay & periodically, thereafter.  Note: this will rollover to 0 eventually so not best way to handle
  if (millis() - tyme > 13000)  //15000)
  {
    println("millis(): " + millis());
    //save millis() in tyme
    tyme = millis();
    //issue I'm Alive msg
    if (LOG_SERIAL)
    {
      println("sending ALIVE msg");
    }
    formALIVE_Msg();
    println("Calling send_TxRequest16(aliveMsg)");
    send_TxRequest16(aliveMsg);
    if (tx_st_response.isSuccess())
    {
      aliveTime = millis();
    }
    delay(2000);
  }  
  //if (LOG_SERIAL)
  //{
  print("millis(): " + millis() + " in draw()...toggleReq[]: ");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    print(toggleReq[i]);
    print(" ");
  }
  println();
  //}
  //transmit toggle requests
  for (int i = 0; i < numCounterMeasures; i++)
  {
    if (LOG_SERIAL)
    {
      println("i: " + i);
    }
    if (toggleReq[i] == true)
    {
      //if (LOG_SERIAL)
      //{
      println("in draw()...entered toggleReq[i] logic with i = " + i);
      println("in draw()...Change to a Switch State " + char(i+49) + " requested");
      //}
      // create a unicast packet to be delivered to remote radio with 16-bit address: 0002, with payload "ALIVE"
      TxRequest16 request = new TxRequest16(new XBeeAddress16(0x00, 0x02), new int[] {
        'T', 'G', 'C', 'M', char(i+49)
      }
      );
      println("in draw()...calling send_TxTequest16 from PIT(main)_REVx_x");
      send_TxRequest16(request);
      checkForReceivedPacket();
      checkForReceivedPacket();
      //reset toggleReq[i]
      toggleReq[i] = false;
      println("in Draw()...we have reset toggleReq[" + i + "] to " + toggleReq[i]);
    }
  }

  //transmit data
  //    send_TxRequest16(request);


  //receive data
  checkForReceivedPacket();
  //render graphic
  // draw the switches on the screen
  for (int i =0; i<switches.size(); i++) {
    //print("i: " + i + "\t");
    ((Switch) switches.get(i)).render();
    //  for (switchCounter =0; switchCounter<switches.size(); switchCounter++) {
    //    print("switchCounter: " + switchCounter + "\t");
    //    ((Switch) switches.get(switchCounter)).render();
  }

  // draw the thermometers on the screen
  for (int j =0; j<indicators.size(); j++) {
    ((Indicator) indicators.get(j)).render();
  }

  update_status_lights();

  // draw the lights on the screen
  for (int j = 0; j<lights.size(); j++)
  {
    ((Light) lights.get(j)).render();
  }

  //write data to output file  
  write_file();
} // end of draw loop

void send_TxRequest16(TxRequest16 _request)
{
  try
  {
    // send the packet and wait up to 12 seconds for the transmit status reply (you should really catch a timeout exception)
    println("in send_TxRequest16...TRANSMITTING NOW!!!");
    tx_st_response = (TxStatusResponse) xbee.sendSynchronous(_request, 12000);
  }
  catch (XBeeTimeoutException e)
  {
    println("in send_TxRequest16...XBeeTimeoutException: " + e);
  }
  catch (XBeeException e)
  {
    println("in send_TxRequest16...XBeeException: " + e);
  }
  if (tx_st_response.isSuccess()) {
    // packet was delivered successfully
    System.out.println("in send_TxRequest16...HURRAY Packet was delivered");
    println("in send_TxRequest16...packet was: " + _request);
  } 
  else {
    // packet was not delivered
    System.out.println("in send_TxRequest16...Packet was not delivered.  status: " + tx_st_response.getStatus());
  }
}

void update_status_lights()
{
  //light motor temperature light if an invalid (out of range) motor temperature is received or if there is a communication error with XBee (receiveError = 1).
  //light controller temperature if an invalid (out of range) controller temperature (controller_temperature_error = 1) is received or if there is a communication error with XBee (receiveError = 1).
  //light communication light if there is a communication error with XBee (receiveError = 1).
  if (LOG_SERIAL)
  {
    println("receiveError, controller_temperature_error, motor_temperature_error: " + receiveError + "\t" + controller_temperature_error + "\t" + motor_temperature_error);
  }
  //set indicator lights to good condition
  //then test for error and set lights accordingly.
  ((Light) lights.get(0)).status = 0;
  if (millis() - aliveTime > 14000)
  {
    ((Light) lights.get(0)).status = 1;
  }
}

// This subroutine accepts the output of the 10 bit ADC and returns the temperature in celcius
// It uses the datasheet for the thermistor.
float temperature_read (float temp_adc_val )
//int temperature_read (int temp_transfer )
{

  float temp_transfer = 0;
  //int temp_adc_val;


  if (temp_adc_val < 217) // (182.6ohms/ (182.6ohms + 680ohms))*3.3V. Convert result to 10bit adc integer. Get 217.
  {
    temp_transfer = 9000; // temperature is over 9000. Out of range of thermistor. Extremely hot.
  }

  else if (temp_adc_val > 1003) // (32554ohms/ (32554ohms + 680ohms))*3.3V. Convert result to 10bit adc integer. Get 1003.
  {
    temp_transfer = 0; // temperature is very cold. Motor is frozen.
  }

  else if ((temp_adc_val >= 997)&&(temp_adc_val < 1003)) // same calculations as above
  {
    temp_transfer = 5; // 5 degrees celsius
  }

  else if ((temp_adc_val >= 990)&&(temp_adc_val < 997))
  {
    temp_transfer = 10; // 10 degrees
  }



  else if ((temp_adc_val >= 981)&&(temp_adc_val < 990))
  {
    temp_transfer = 15;
  }



  else if ((temp_adc_val >= 971)&&(temp_adc_val < 981))
  {
    temp_transfer = 20;
  }

  else if ((temp_adc_val >= 959)&&(temp_adc_val < 971))
  {
    temp_transfer = 25;
  }


  else if ((temp_adc_val >= 944)&&(temp_adc_val < 959))
  {
    temp_transfer = 30;
  }

  else if ((temp_adc_val >= 927)&&(temp_adc_val < 944))
  {
    temp_transfer = 35;
  }

  else if ((temp_adc_val >= 908)&&(temp_adc_val < 927))
  {
    temp_transfer = 40;
  }


  else if ((temp_adc_val >= 886)&&(temp_adc_val < 908))
  {
    temp_transfer = 45;
  }

  else if ((temp_adc_val >= 861)&&(temp_adc_val < 886))
  {
    temp_transfer = 50;
  }
  else if ((temp_adc_val >= 834)&&(temp_adc_val < 861))
  {
    temp_transfer = 55;
  }
  else if ((temp_adc_val >= 804)&&(temp_adc_val < 834))
  {
    temp_transfer = 60;
  }
  else if ((temp_adc_val >= 772)&&(temp_adc_val < 804))
  {
    temp_transfer = 65;
  }
  else if ((temp_adc_val >= 738)&&(temp_adc_val < 772))
  {
    temp_transfer = 70;
  }
  else if ((temp_adc_val >= 702)&&(temp_adc_val < 738))
  {
    temp_transfer = 75;
  }
  else if ((temp_adc_val  >= 664)&&(temp_adc_val  < 702))
  {
    temp_transfer = 80;
  }
  else if ((temp_adc_val  >= 626)&&(temp_adc_val  < 664))
  {
    temp_transfer = 85;
  }
  else if ((temp_adc_val  >= 588)&&(temp_adc_val < 626))
  {
    temp_transfer = 90;
  }
  else if ((temp_adc_val >= 549)&&(temp_adc_val  < 588))
  {
    temp_transfer = 95;
  }
  else if ((temp_adc_val  >= 511)&&(temp_adc_val  < 549))
  {
    temp_transfer = 100;
  }
  else if ((temp_adc_val  >= 474)&&(temp_adc_val  < 511))
  {
    temp_transfer = 105;
  }
  else if ((temp_adc_val  >= 438)&&(temp_adc_val  < 474))
  {
    temp_transfer = 110;
  }
  else if ((temp_adc_val  >= 404)&&(temp_adc_val  < 438))

  {
    temp_transfer = 115;
  }
  else if ((temp_adc_val  >= 371)&&(temp_adc_val  < 404))
  {
    temp_transfer = 120;
  }
  else if ((temp_adc_val  >= 340)&&(temp_adc_val  < 371))
  {
    temp_transfer = 125;
  }
  else if ((temp_adc_val  >= 312)&&(temp_adc_val  < 340))
  {
    temp_transfer = 130;
  }
  else if ((temp_adc_val  >= 285)&&(temp_adc_val  < 312))
  {
    temp_transfer = 135;
  }
  else if ((temp_adc_val >= 260)&&(temp_adc_val < 285))
  {
    temp_transfer = 140;
  }
  else if ((temp_adc_val>= 238)&&(temp_adc_val < 260))
  {
    temp_transfer = 145;
  }

  else if ((temp_adc_val >= 217)&&(temp_adc_val < 238))
  {
    temp_transfer = 150;
  }

  else 
  {
    motor_temperature_error = 1;
  }

  //  ((Light) lights.get(0)).status = motor_temperature_error;
  return temp_transfer ;
}

// this function runs once every time the mouse is pressed
void mousePressed() {
  // check every switch object on the screen to see 
  // if the mouse press was within its borders
  // and toggle the state if it was (turn it on or off)
  //  for (int i=0; i < switches.size(); i++) {
  //    ((Switch) switches.get(i)).toggleState();
  //  }
  for (switchCounter=0; switchCounter < switches.size(); switchCounter++)
  {
    ((Switch) switches.get(switchCounter)).toggleState();
  }
}

void write_file()
{
  if (LOG_SERIAL)
  {
    println("(((Switch) switches.get(0)).state) from Telemetry: " + (((Switch) switches.get(0)).state));
  }

  //Routine to print data in each XBee array as hex characters
  //Data goes to PC terminal emulator
  XBee_numb = 0;
  DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd@HH_mm_ss");
  Date d = new Date();
  String time = formatter.format(d);
  output.print(time + ":");
  output.print(" MT: " + ((Indicator) indicators.get(0)).temp);
  output.print(",");
  output.print(" CT: " + ((Indicator) indicators.get(1)).temp);
  output.print(",");
  output.print(" TP: " + ((Indicator) indicators.get(2)).temp);
  output.print(",");
  output.print(" CV: " + ((Indicator) indicators.get(3)).temp);
  output.print(",");
  output.print(" CC: " + ((Indicator) indicators.get(4)).temp);
  output.print(",");
  output.print(" Lights: ");
  output.print(((Light) lights.get(0)).status);
  output.print(",");
  output.print(" Switches: ");
  output.print(((Switch) switches.get(0)).state);
  output.print(",");
  output.print(((Switch) switches.get(1)).state);
  output.print(",");
  output.print(((Switch) switches.get(2)).state);
  output.print(",");
  output.print(((Switch) switches.get(3)).state);
  output.println();
}


//The record format comes from Geno's Arduino on the CAR.
//It contains motor control voltage (float), motor controller current (float), 
//motor controller temperature (short), and throttle position (short).
//This code came from Anthony Cascone.

//In response to my Tx64 PB command, I will receive a record
//from Geno's arduino that will look something like this
//7E 00 19 80 00 13 A2 00 40 9C 9D A6 00 04 50 42 00 00 11 11 22 22 33 33 44 44 55 55 17
// where: 50 42 = PB in hex
//        00 00 = status of car's Arm0 indicator in hex
//        11 11 = status of car's Arm1 indicator in hex
//        00 00 = status of car's Arm2 indicatorin hex
//        00 00 = status of car's Arm3 indicator in hex
//        00 00 = status of car's Fire indicator in hex
//        00 00 = status of car's Pit_In indicator in hex
//
//In response to my Tx64 msg, I will receive a record
//from Geno's arduino that will look something like this
//7E 00 13 80 00 13 00 A2 40 9C 92 A6 00 00 46 00 11 22 33 44 55 66 0B
void check_80_record()
{
  if (LOG_SERIAL)
  {
    println("checking 80 record");
  }

  short controller_temp;
  float controller_voltage;
  float controller_current;
  short controller_throttle_position;
  byte data[] = new byte[] {
    (byte)0x99, (byte)0x99, (byte)0x65, (byte)0x41, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x18, (byte)0x00, (byte)0xFF, (byte)0x00
  };
  byte PB_data[] = new byte[] 
  {
  };
  if (dataIS[XBee_numb][2] == 0x13 & dataIS[XBee_numb][14] == 0x50 & dataIS[XBee_numb][15] == 0x42)  // data came from Geno's arduino in response to my Tx64 PB command
  {
  }
  else       if (dataIS[XBee_numb][2] == 0x19 & dataIS[XBee_numb][14] == 0x46)  // data came from Geno's arduino in response to my Tx64 ALIVE msg

  {
    ((Switch) switches.get(0)).state = false;
    ((Switch) switches.get(1)).state = true;
    ((Switch) switches.get(2)).state = false;
    ((Switch) switches.get(3)).state = false;
    ((Switch) switches.get(4)).state = false;
    ((Switch) switches.get(5)).state = false;
  }
  else  //data from Geno came from motor controller...Use Anthony's code
  {
    //get bytes from dataIS array that correspond to voltage, current, temperature, and throttle position.
    for (int i = 0; i < 12; i++)
    {
      data[i] = (byte)dataIS[XBee_numb][bytecount-12+i];
      if (LOG_SERIAL)
      {
        println(hex(data[i], 2));
      }
    }

    //NOTE TO SELF:  HAVE TO CHANGE REFERENCES TO INDICATORS WHEN AN INDICATOR IS DELETED!!!
    ByteBuffer buffer = ByteBuffer.wrap(data);
    buffer.order(java.nio.ByteOrder.LITTLE_ENDIAN);
    //      if (LOG_SERIAL)
    //      {
    //    println(controller_voltage = buffer.getFloat());
    //      }
    //    ((Indicator) indicators.get(3)).temp = controller_voltage;
    //      if (LOG_SERIAL)
    //      {
    //    println(controller_current = buffer.getFloat());
    //      }
    //    ((Indicator) indicators.get(4)).temp = controller_current; 
    //      if (LOG_SERIAL)
    //      {
    //    println(controller_temp = buffer.getShort());
    //      }
    //    ((Indicator) indicators.get(2)).temp = controller_temp;
    //      if (LOG_SERIAL)
    //      {
    //    println(controller_throttle_position = buffer.getShort());
    //      }
    //    ((Indicator) indicators.get(3)).temp = controller_throttle_position;
  }
}  //end check_80_record function

void keyPressed() {
  output.flush(); // Writes the remaining data to the file
  output.close(); // Finishes the file
  println("Exiting program");
  exit(); // Stops the program
}

void getResponse1()
{
  if (LOG_SERIAL)
  {
    println("in getResponse1()");
  }
  try {
    response = xbee.getResponse(10000);
    // we got a response!
    if (LOG_SERIAL)
    {
      println("we got a response!");
      println("response.getApiId(): " + response.getApiId());
    }
  } 
  catch (XBeeTimeoutException e) {
    if (LOG_SERIAL)
    {
      println("we timed out without a response");
    }
    // we timed out without a response
  }
  catch (XBeeException e)
  {
  }

  if (response.getApiId() == ApiId.RX_16_RESPONSE)
  {
    // since this API ID is RX_16_RESPONSE, we know to cast to 16
    rxResponse = (RxResponse16) response;
    options = rxResponse.getOptions();
    if (LOG_SERIAL)
    {
      println("RX_16_RESPONSE received with options: " + options);
    }
    data = rxResponse.getData();
    if (LOG_SERIAL)
    {
      for (int i = 0; i < data.length; i++)
      {
        println("data[" + i + "]" + " " + char(data[i]));
      }
    }
  }
  else if (response.getApiId() == ApiId.RX_16_IO_RESPONSE)
  {
    if (LOG_SERIAL)
    {
      println("ApiId.RX_16_IO_RESPONSE recognized");
    }

    // since this API ID is RX_16_IO_RESPONSE, we know to cast to RxResponseIoSample
    RxResponseIoSample rxResponse16 = (RxResponseIoSample) response;
    if (LOG_SERIAL)
    {
      println("Got an RX_16_IO_RESPONSE...rxResponse16 has been made");
    }
    ioData = rxResponse16.getSamples();
    if (LOG_SERIAL)
    {
      println("ioData.length is: " + ioData.length);
      for (int i = 0; i < ioData.length; i++)
      {
        println("ioData[" + i + "] is: " + ioData[i]);
      }
    }
    //    int value = ioData[0].getAnalog0();
    int value = ioData[0].getAnalog1();
    //    println("Analog0 value: " + value);
    if (LOG_SERIAL)
    {
      println("Analog1 value: " + value);
    }
    ((Indicator) indicators.get(0)).temp = (value);
    analog_voltage = value * Vref / 1024;
    if (LOG_SERIAL)
    {
      //    print("A0 Voltage:  ");
      print("A1 Voltage:  ");
      print(analog_voltage);
      print("\n\r");
    }
  }

  write_file();
}

