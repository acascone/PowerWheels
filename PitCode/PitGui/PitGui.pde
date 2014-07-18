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
  
 */

import java.util.*;
import java.text.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.lang.Object;

import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.PacketListener;
import com.rapplogic.xbee.api.wpan.TxStatusResponse;

// TODO: These should be added to a menu item at somepoint
final boolean LOG_SERIAL = false;
final boolean RUN_OFFLINE = false;

final int LIVE_TMO_MS = 2000;
final int numCounterMeasures = 4;  //number of counter measures on CAR

// create and initialize a new xbee object
XBee xbee = new XBee();
int[] data; // TODO: remove

RxResponse16 rxResponse; // TODO: remove
TxRequest16 request; // TODO: remove
TxRequest16 aliveMsg = new TxRequest16(new XBeeAddress16(0x00, 0x02), new int[] { 'A', 'L', 'I', 'V', 'E' }  );

ArrayList<Light> lights;  //an arrayList for all the lights
ArrayList<Switch> switches;  //an arrayList for all the switches
ArrayList<Indicator> indicators;
boolean[] toggleReq = new boolean[numCounterMeasures];  //boolean array used to keep track of which counter measure switch is being toggled

PFont font; // create a font for display
PImage img;

final String MotorTemp      = "M";  //motor1 temp
final String ControllerTemp = "C";  //controller temp
final String MotorTemp2     = "N";  //motor2 temp
final String ThrottlePos    = "T";  //throttle pos
final String Voltage        = "V";  //voltage 
final String Current        = "I";  //current (amps)

PrintWriter output;

//Define ADC reference-voltage input, change as needed.
final float Vref = 3.3;
float   analog_voltage;
int motorControlTemperatureValue = 0;    //variable contains motor control analog temp value
int     pLoad = 0;
int     aliveTime;    //variable to store time of successful response from Alive msg.

// Class for talking to G35LightBoard
G35LightBoard networkedDisplay;

void checkForReceivedPacket(XBeeResponse response)
{
    // we got a response!
    if (LOG_SERIAL)
    {
      println("in checkForReceivedPacket()...we got a response!");
      println("in checkForReceivedPacket()...response.getApiId(): " + response.getApiId());
    }

    if (response.getApiId() == ApiId.RX_16_RESPONSE)
    {
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
        pLoad += pow(10, (4-i)) * (data[i] - '0');
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
          } else
          {
            st8 = true;
          } 
          switches.get(data[2] - '1').state = st8;
          int swNum = data[2] - '1';
          if (LOG_SERIAL)
          {
            println("saw " + (char)data[0] + (char)data[1] + (char)data[2] + (char)data[3] + (char)data[4]);
            println("setting counter measure sw" + swNum + " " + (char)data[3] + (char)data[4]);
          }
        }
        if (data[0] == 'T')
        {
          ((Indicator) indicators.get(1)).temp = pLoad;
        } else if (data[0] == 'P')
        {
          ((Indicator) indicators.get(2)).temp = pLoad;
        } else if (data[0] == 'V')
        {
          ((Indicator) indicators.get(3)).temp = pLoad;
        } else if (data[0] == 'I')
        {
          ((Indicator) indicators.get(4)).temp = pLoad;
        } else if (data[0] == 'M')
        {
          ((Indicator) indicators.get(0)).temp = pLoad;
        } else
        {
          //we got an unexpected RX_16_RESPONSE
        }
      }
    }
    if (response.getApiId() == ApiId.RX_16_IO_RESPONSE)
    {
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

      for (IoSample sample : ioSample.getSamples ())
      {          
        if (ioSample.containsAnalog())
        {
          System.out.println("Analog pin 20 10-bit reading is " + sample.getAnalog0());
          System.out.println("Analog pin 19 10-bit reading is " + sample.getAnalog1());
          motorControlTemperatureValue = sample.getAnalog1();
          System.out.println("Analog pin 18 10-bit reading is " + sample.getAnalog2());
        } else
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

void setup()
{
  networkedDisplay = new G35LightBoard(this, "alarmpi.home.lan");
  networkedDisplay.start();
  
  size(1200, 850); // screen size
  smooth(); // anti-aliasing for graphic display

  aliveTime = millis() - LIVE_TMO_MS;
  
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
  
  if (!RUN_OFFLINE)
  {
    try
    {
      //xbee.open("COM12", 9600);
      xbee.open("/dev/tty.usbserial-A1014K3A", 9600);
      
      xbee.addPacketListener(new PacketListener() {
      public void processResponse(XBeeResponse response) {
          if (response.getApiId() != ApiId.TX_STATUS_RESPONSE)
          {
            aliveTime = millis();
          }
          checkForReceivedPacket(response);
      }
      });
    }
    catch (XBeeException e)
    {
      // display the exception
      println(e);
      
      // handle the exception
      exit();
    }
  }
  
  // Create a new file in the sketch directory to store the data coming from the Power Wheels CAR
  DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd@HH_mm_ss");
  Date d = new Date();
  String fileName = formatter.format(d);
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
  indicators.add(new Thermometer(MotorTemp, 35, 450, 0 * 75 + 40, 20, "Motor Temperature", "˚F", 0, 300));
  //  indicators.add(new Thermometer(MotorTemp2, 35, 450, 1 * 75 + 40, 20, "Motor2 Temperature", "˚F", 0, 1023));
  indicators.add(new Thermometer(ControllerTemp, 35, 450, 2 * 75 + 40, 20, "Controller Temperature", "˚F", 0, 100));
  indicators.add(new AnalogMeter(ThrottlePos, 450, 40, 375, 140, "Throttle Position", "%", 0, 100));
  indicators.add(new VoltMeter(Voltage, 450, 40, 375, 240, "Voltage", "Volts", 0, 45));
  indicators.add(new AnalogMeter(Current, 450, 40, 375, 340, "Current", "Amps", 0, 300));
  
  lights = new ArrayList<Light>();
  lights.add(new Light("Alive ACK", 100, 100, 3*140 + 40, 500, 0));

  img = loadImage("wide_HackPGH_Logo.png");

  //initialize toggleReq array
  for (int i = 0; i < numCounterMeasures; i++)
  {
    println("initializing toggleReq[" + i + "]");
    toggleReq[i] = false;
  } 
}  //END setup()

// draw loop executes continuously
void draw()
{
  
  //println("in Draw() " + millis());
  
  background(255); // draw a white background
  imageMode(CORNERS);
  image(img, width-200, 0, width, 200);
  
  if (LOG_SERIAL)
  {
    print("millis(): " + millis() + " in draw()...toggleReq[]: ");
    for (int i = 0; i < numCounterMeasures; i++)
    {
      print(toggleReq[i]);
      print(" ");
    }
    println();
  }
  
  // transmit toggle requests
  for (int i = 0; i < numCounterMeasures; i++)
  {
    if (LOG_SERIAL)
    {
      println("i: " + i);
    }
    if (toggleReq[i] == true)
    {
      if (LOG_SERIAL)
      {
        println("in draw()...entered toggleReq[i] logic with i = " + i);
        println("in draw()...Change to a Switch State " + char(i+49) + " requested");
      }
      
      TxRequest16 request = new TxRequest16(new XBeeAddress16(0x00, 0x02), new int[] { 'T', 'G', 'C', 'M', char(i+49) } );
      
      try {
        xbee.sendAsynchronous(request);
      } catch (Exception e) {};
      
      // perhaps we should set this to false after we got the response from the car
      toggleReq[i] = false;
      println("in Draw()...we have reset toggleReq[" + i + "] to " + toggleReq[i]);
    }
  }

  //render graphic
  // draw the switches on the screen
  for (int j = 0; j < switches.size(); j++) {
    switches.get(j).render();
  }

  // draw the thermometers on the screen
  for (int j = 0; j < indicators.size(); j++) {
    indicators.get(j).render();
  }

  update_status_lights();

  // draw the lights on the screen
  for (int j = 0; j < lights.size(); j++)
  {
    lights.get(j).render();
  }

  //write data to output file  
  write_file(); 
} // end of draw loop

void update_status_lights()
{
  //set indicator lights to good condition
  //then test for error and set lights accordingly.
  if (abs(millis() - aliveTime) > LIVE_TMO_MS)
  {
    lights.get(0).status = 1;
  }
  else
  {
    lights.get(0).status = 0;
  }
}

// this function runs once every time the mouse is pressed
void mousePressed() {
  // check every switch object on the screen to see 
  // if the mouse press was within its borders
  // and toggle the state if it was (turn it on or off)
  for (int j=0; j < switches.size (); j++)
  {
    if (switches.get(j).checkSwitch())
    {
      // this switch was clicked
      toggleReq[j] = true;
      // we can break the loop since we can only have one mouse press 
      // at a time. this may not hold true for multitouch interfaces.
      break;
    }
  }
}

void write_file()
{
  if (LOG_SERIAL)
  {
    println("(switches.get(0).state) from Telemetry: " + (switches.get(0).state));
  }

  //Routine to print data in each XBee array as hex characters
  //Data goes to PC terminal emulator
  DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd@HH_mm_ss");
  Date d = new Date();
  String time = formatter.format(d);
  output.print(time + ":");
  output.print(" MT: " + indicators.get(0).temp + ",");
  output.print(" CT: " + indicators.get(1).temp + ",");
  output.print(" TP: " + indicators.get(2).temp + ",");
  output.print(" CV: " + indicators.get(3).temp + ",");
  output.print(" CC: " + indicators.get(4).temp + ",");
  output.print(" Lights: ");
  output.print(lights.get(0).status + ",");
  output.print(" Switches: ");
  output.print(switches.get(0).state + ",");
  output.print(switches.get(1).state + ",");
  output.print(switches.get(2).state + ",");
  output.print(switches.get(3).state);
  output.println();
}

void keyPressed() {
  output.flush(); // Writes the remaining data to the file
  output.close(); // Finishes the file
  println("Exiting program");
  exit(); // Stops the program
}

// This is an experimental function to see if we catch an exit on the window close.
// This may not work, but it's worth testing
void exit()
{
  println("In stop");
  try
  {
    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
    networkedDisplay.Close();
  }
  catch (Exception e) {}
  super.exit();
}

