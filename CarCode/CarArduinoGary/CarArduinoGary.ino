/*NOTE:CHANGE FOR GITHUB
 *20140511  GEM  CAR_REV1_0
 *Start with CAR_REV0_0
 *
 *20140528  GEM  CAR_REV2_0
 *Add generation of throttle position, voltage, current
 *
 *20140529  GEM  CAR_REV2_1
 *Transmit throttle position, voltage, current once every loopsUntilXmit times (e.g. once ever 5 looptimes...loopUntilXmit = 5;)
 *
 *20140531  GEM  CAR_REV3_0
 *Debug s/w cmPB latency issue
 *
 *20140531  GEM  CAR_REV3_1
 *Lots of cmPB latency issues.
 *Trying to get s/w switches to follow h/w LEDs.
 *Probably need to re-write transmit functions
 *
 *20140602  GEM  CAR_REV4_0
 *TransmitData_REV1_0.ino seemed to have no latency issues.
 *Use checkForRcvdPacket() function from TransmitData_REV1_0.ino for receiving packets
 *
 *20140606  GEM  CAR_REV5_0
 *Change checkForRcvdPacket() function to follow isAvailable & isError logic from Series1_Rx in XBee examples folder
 *
 *20140608  GEM  CAR_REV6_0
 *Add logic to provide capability to handle toggle requests
 *& ALIVE transmission from PIT.
 *This logic will be similar to logic from getResponses().
 *
 *20140608  GEM  CAR_REV6_1
 *When ALIVE transmission is received from PIT, 
 *CAR must respond with cmLED_State_Now[] 
 *so that PIT processing S/W can mirror the correct states.
 *
 *20140609  GEM  CAR_REV6_2
 *Randomize voltage, throttle position, current
 *
 *20140615  GEM  CAR_REV6_3
 *In addition to voltage, throttle position, current, add controller temp
 *
 *20140616  GEM  CAR_REV7_0
 *Change motor temperature...It was on XBee pin 19.  Change it to be on analog input to Arduino pin A0.
 *
 *20140616  GEM  CAR_REV7_1
 *Change motor temperature...convert ADC units to degC.
 *
 *20140622  GEM  CAR_REV7_2
 *Add capability to handle FIRE & PIT IN switches from PIT.
 *
 *20140624  GEM  CAR_REV7_3
 *Add logic to FIRE counter measures.
 *Add logic to light PIT IN light.
 *
 */
#include <XBee.h>
#include <NewSoftSerial.h>

//#define LOG_SERIAL    //log to serial device when uncommented

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();
Rx16IoSampleResponse ioSample = Rx16IoSampleResponse();

//**********
//NOTE:  20140624:  So that XBee did not have to be re-programmed, we will
// use pin3 as FIRE and pin4 as PIT IN, so flip notes below
boolean pin3;  //stores status of PIT XBee AD03 (PIT IN)
boolean pin4;  //stores status of PIT XBee AD04 (FIRE)
//**********

int statusLed = 11;
int errorLed = 12;
int dataLed = 10;

uint8_t option = 0;
uint8_t data = 0;
const int numCounterMeasures = 4;
int cmLED_Pins[numCounterMeasures] = {
  13, 12, 5, 7};      // pins for counter measure LEDs
int *ptr_cmLED_Pins;          // pointer to cmLED_Pins[]
int cmPB_Pins[numCounterMeasures] = {
  4, 6, 8, 9};  //pins for counter measure arming PBs
int PIT_IN_LED = 2;    //PIT IN LED is on Arduino pin 2
int cm_Pins[3] = {
  3, 10, 11};  //pins for counter measure PBs
int *ptr_cmPB_Pins;
boolean bool_cmPB_State_Now[numCounterMeasures] = {
  false, false, false, false};        // current state of counter measure PBs
boolean bool_cmPB_State_Prev[numCounterMeasures] = {
  false, false, false, false};        // previous state of counter measure PBs
int cmPB_State_Now[numCounterMeasures] = {
  0, 0, 0, 0};        // current state of counter measure PBs
int cmPB_State_Prev[numCounterMeasures] = {
  0, 0, 0, 0};        // previous state of counter measure PBs
int cmLED_State_Now[numCounterMeasures] = {
  0, 0, 0, 0};        // current state of counter measure LEDs
int cmLED_State_Prev[numCounterMeasures] = {
  0, 0, 0, 0};        // previous state of counter measure LEDs
boolean toggleReq[] = {
  false, false, false, false};
boolean aliveReq = false;
// pitUp variable indicates status of pit XBee/processing system...
// true = pit system up & working, false = not working
boolean pitAlive       = false;
boolean pitAliveNow    = false;
boolean pitAlivePrev   = false;
boolean pitAlivePulse  = false;
boolean pitAliveMsgRcvd = false;


unsigned long start = millis();

// allocate five bytes for to hold a payload
uint8_t payload[] = { 
  0, 0, 0, 0, 0};

// with Series 1 you can use either 16-bit or 64-bit addressing
// 16-bit addressing: Enter address of remote XBee, typically the coordinator
//Tx16Request tx = Tx16Request(0x1874, payload, sizeof(payload));
Tx16Request tx = Tx16Request(0x0005, payload, sizeof(payload));
TxStatusResponse txStatus = TxStatusResponse();

int pin5 = 0;

int motorTemperaturePin      = A0;  //A0 is pin that connects to motor thermistor
int motorTemperatureValueADC = 0;   //variable to store the ADC value coming from the motor thermistor


//Start Series1_Tx16Request_REV002 change
// constants won't change. Used here to 
// set pin numbers:

// Variables will change:
int cm1LED_State         = LOW;      // variable for LED state for counter measure 1
long previousMillis      = 0;        // will store last time LED was updated
//int cm1PB_State        = 0;        // variable for PB state for counter measure 1 pushbutton
int cm1PB_State_Now      = 0;        // variable for current state of PB for counter measure 1 pushbutton
int cm1PB_State_Prev     = 0;        // variable for previous state of PB for counter measure 1 pushbutton
int pLoad = 0;                        //variable used to convert character data to decimal
int throttlePosition = 0;
int voltage = 0;
int current = 0;
int controllerTemperature = 0;
int loopsUntilXmit = 5;  //transmit throttle position, voltage, current, and controller temperature once every 5 loops
int loopCounter = 0;      //Counts up to loopsUntilXmit, then resets to 0 when analog values are transmitted 

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)
//End Series1_Tx16Request_REV002 change

void convertCharDataToDecimal()
{
  for (int i = 1; i < 5; i++)
  {
    payload[i] = pLoad / pow(10, (4 - i));
    pLoad -= payload[i] * pow(10, (4 - i));
    payload[i] += 48;
  }

}

void handle_RX_16_RESPONSE()
{
  //Valid RX_16_RESPONSE should have rx16.getDataLength() = 5
  //and rx16.getData(4) will hold either an "E" (ALIVE)
  //or the number 1 through 4 if PIT sends a toggle request (TGCM1, TGCM2, TGCM3, TGCM4).
  if (rx16.getDataLength() == 5)
  {
#ifdef LOG_SERIAL
    Serial.print("in void getResponses()...rx16.getData(4): ");
    Serial.println(rx16.getData(4));
#endif
    if (rx16.getData(4) == 'E')
    {
#ifdef LOG_SERIAL
      Serial.println("in void getResponses()...setting pitAliveNow true");
#endif
      pitAliveMsgRcvd = true;  //when true, a new pit ALIVE msg has been received from PIT
      pitAliveNow = true;  //when true, pit ALIVE msg has been received within the past "x" seconds; pit ALIVE msg is considered current
#ifdef LOG_SERIAL
      Serial.print("pitAliveNow: ");
      Serial.println(pitAliveNow);
#endif
    }
    //        else if (rx16.getData(4) >= 0 && rx16.getData(4) <= 4)
    //        else if (rx16.getData(4) >= 0 + 31 + 19 && rx16.getData(4) <= 4 + 31 + 19)
    else if (rx16.getData(4) >= '1' && rx16.getData(4) <= '4')
    {
      toggleReq[(char)rx16.getData(4) - 49] = true;
#ifdef LOG_SERIAL
      Serial.print("We have set toggleReq ");
      Serial.print((char)rx16.getData(4) - 49);
      Serial.print(" to: ");
      Serial.println(toggleReq[(char)rx16.getData(4) - 49]);
      Serial.println(" NOTE this is not LED state");
#endif
    }
    else
    {
#ifdef LOG_SERIAL
      Serial.println("We received an RX_16_RESPONSE of length 5 BUT data in rx16.getData(4) was invalid");
#endif
    }
  }  // end       if (rx16.getDataLength() == 5)

  else    //else error
  {
#ifdef LOG_SERIAL
    Serial.println("We received a RX_16_RESPONSE BUT it was the wrong length (not of length 5)");
#endif
  }

}

void checkForRcvdPacket()
{
  //  if (xbee.readPacket(5000))
  //  {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable())
  {
    //          #ifdef LOG_SERIAL
    Serial.println("***in void checkForRcvdPacket()***");
    Serial.print("got response: 0x");
    Serial.print(xbee.getResponse().getApiId(), HEX);
    Serial.print(" at approx. time: ");
    Serial.println(millis());
    //          #endif

    switch (xbee.getResponse().getApiId())
    {
    case  TX_STATUS_RESPONSE:
      //***************
      //NOTE:  We probably got here because ALIVE msg 
      //was received by CAR in the middle of a transmission by CAR.
      //***************

      //Even though the following line of code references a Series2 function, 
      //it is needed for Series1 XBees to get status of response 
      //which is tested for SUCCESS below.
      xbee.getResponse().getZBTxStatusResponse(txStatus);
#ifdef LOG_SERIAL
      Serial.print("in checkForRcvdPacket()...got TX_STATUS_RESPONSE ");
#endif

      // get the delivery status, the fifth byte
      if (txStatus.getStatus() == SUCCESS)
      {
        // success.  time to celebrate
#ifdef LOG_SERIAL
        Serial.println("...GOOD status");
#endif
      } 
      else
      {
        // the remote XBee did not receive our packet. is it powered on?
#ifdef LOG_SERIAL
        Serial.println("...BAD status...is remote XBee powered on?");
#endif
      }
      break;
    case  RX_16_RESPONSE:
#ifdef LOG_SERIAL
      Serial.print("at time (millis()): ");
      Serial.print(millis());
      Serial.print(" ");
      Serial.println("we got an RX_16_RESPONSE ");

      //get the error code
      Serial.println("the following error code msg prints all the time");
      Serial.print("xbee.getResponse().getErrorCode(): ");
      Serial.println(xbee.getResponse().getErrorCode());
#endif

      xbee.getResponse().getRx16Response(rx16);
#ifdef LOG_SERIAL
      for (int i = 0; i < rx16.getDataLength(); i++)
      {
        Serial.print("i: ");
        Serial.print(i);
        Serial.print("\t"); 
        Serial.println(char(rx16.getData(i)));
      }
#endif
      handle_RX_16_RESPONSE();
      break;
    case RX_16_IO_RESPONSE:
      //#ifdef LOG_SERIAL
      Serial.println("***in void checkForRcvdPacket()***");
      Serial.print("at time (millis()): ");
      Serial.print(millis());
      Serial.print(" ");
      Serial.println("we got an RX_16_IO_RESPONSE ");
      //#endif

      xbee.getResponse().getRx16IoSampleResponse(ioSample);
      //      if (boolean pin3 = ioSample.isDigitalEnabled(3))
      pin3 = ioSample.isDigitalEnabled(3);
      pin4 = ioSample.isDigitalEnabled(4);
      {
        //        #ifdef LOG_SERIAL
        Serial.print("pin3: ");
        Serial.println(pin3);
        Serial.print("pin4: ");
        Serial.println(pin4);
        //        #endif
      }
      if (pin4)
      {
        digitalWrite(PIT_IN_LED, HIGH);
        delay(2000);
      }
      else
      {
        digitalWrite(PIT_IN_LED, LOW);
      }
if (pin3)
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(cm_Pins[i], LOW);
  }
}
else
//      if (!pin3)
      {
        Serial.print("cm_Pins[0/1/2]: ");
        for (int i = 0; i < 3; i++)
        {
          digitalWrite(cm_Pins[i], cmLED_State_Now[i]);

          //  #ifdef LOG_SERIAL
          Serial.print(cmLED_State_Now[i] * !pin3);
          Serial.print(" ");
          //  #endif
        }
        for (int i = 0; i < 3; i++)
        {
          if (cmLED_State_Now[i] == 1)
          {

            //            #ifdef LOG_SERIAL
//            Serial.println();
//            Serial.print("changing cmLED_State_Now[");
//            Serial.print(i);
//            Serial.print("] to: ");
//            Serial.println(!cmLED_State_Now[i]);
            //            #endif

            cmLED_State_Now[i] = 0;
            digitalWrite(cmLED_Pins[i], cmLED_State_Now[i]);

            //            #ifdef LOG_SERIAL
            Serial.println();
            Serial.println("***in void checkForRcvdPacket()***");
            Serial.print("we have performed a digitalWrite to pin ");
            Serial.print(cmLED_Pins[i]);
            Serial.print(" and have written the value ");
            Serial.println(cmLED_State_Now[i]);
            //            #endif
          }
        }
      }

      //  #ifdef LOG_SERIAL
      Serial.println();
      //  #endif

      break;
    default:
      // statements
      //        #ifdef LOG_SERIAL
      Serial.println("default condition");
      //        #endif
    }    //end switch (xbee.getResponse().getApiId())
  }
  else if (xbee.getResponse().isError())
  {
#ifdef LOG_SERIAL
    Serial.println("if this msg prints, then there must have been a transmission error");
    Serial.print("xbee.getResponse().getErrorCode(): ");
    Serial.println(xbee.getResponse().getErrorCode());
#endif
  }
  else
  {
#ifdef LOG_SERIAL
    Serial.println("XBee port is NOT available");
#endif
  }
  // }    //end if (xbee.readPacket(5000))
}

void send_tx()
{

#ifdef LOG_SERIAL
  Serial.print("sizeof(payload): ");
  Serial.println(sizeof(payload));
  for (int i = 0; i < sizeof(payload); i++)
  {
    Serial.print(" ");
    Serial.print(payload[i]);
  }
  Serial.println();
#endif

  xbee.send(tx);

  // after sending a tx request, we expect a status response
  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
#ifdef LOG_SERIAL
      Serial.println("in void send_tx()...got znet tx status");
#endif


      // get the delivery status, the fifth byte
      if (txStatus.getStatus() == SUCCESS) {
        // success.  time to celebrate
#ifdef LOG_SERIAL
        Serial.println("good status");
#endif

      } 
      else {
        // the remote XBee did not receive our packet. is it powered on?
#ifdef LOG_SERIAL
        Serial.println("bad status");
#endif

      }
    }  //end     if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE)

  }  //end   if (xbee.readPacket(5000))

  else if (xbee.getResponse().isError()) {
    // or flash error led
#ifdef LOG_SERIAL
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
#endif
  } 
  else {
    // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
#ifdef LOG_SERIAL
    Serial.println("no timely TX Status Response...check that CAR serial port is plugged in");
#endif
  }
}

void getResponses()
{
  // Get responses
  xbee.readPacket();

  if (xbee.getResponse().isAvailable())
  {
    Serial.println();
    Serial.println("Got an XBee response");
    Serial.print("XBee response is: ");
    Serial.println(xbee.getResponse().getApiId(),HEX);

    // got something
    // got an rx packet
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE)
    {
      xbee.getResponse().getRx16Response(rx16);
      //        option = rx16.getOption();
      //        data = rx16.getData(0);
#ifdef LOG_SERIAL
      for (int i = 0; i < rx16.getDataLength(); i++)
      {
        Serial.print("i: ");
        Serial.print(i);
        Serial.print("\t"); 
        Serial.println(rx16.getData(i),HEX);
      }
#endif
      //Valid RX_16_RESPONSE should have rx16.getDataLength() = 5
      //and rx16.getData(4) will hold either an "E" (ALIVE)
      //or the number 1 through 4 if PIT sends a toggle request (TGCM1, TGCM2, TGCM3, TGCM4).
      if (rx16.getDataLength() == 5)
      {
#ifdef LOG_SERIAL
        Serial.print("in void getResponses()...rx16.getData(4): ");
        Serial.println(rx16.getData(4));
#endif
        if (rx16.getData(4) == 'E')
        {
#ifdef LOG_SERIAL
          Serial.println("in void getResponses()...setting pitAliveNow true");
#endif
          pitAliveNow = true;
#ifdef LOG_SERIAL
          Serial.print("pitAliveNow: ");
          Serial.println(pitAliveNow);
#endif

        }
        //        else if (rx16.getData(4) >= 0 && rx16.getData(4) <= 4)
        //        else if (rx16.getData(4) >= 0 + 31 + 19 && rx16.getData(4) <= 4 + 31 + 19)
        else if (rx16.getData(4) >= '1' && rx16.getData(4) <= '4')
        {
#ifdef LOG_SERIAL
          Serial.print("about to set toggleReq ");
          Serial.print((char)rx16.getData(4) - 49);
          Serial.println(" true...NOTE this is not LED state");
#endif
          toggleReq[(char)rx16.getData(4) - 49] = true;
        }
        else
        {
#ifdef LOG_SERIAL
          Serial.println("We received an RX_16_RESPONSE of length 5 BUT data in rx16.getData(4) was invalid");
#endif
        }
      }  // end       if (rx16.getDataLength() == 5)

      else    //else error
      {
#ifdef LOG_SERIAL
        Serial.println("We received a RX_16_RESPONSE BUT it was the wrong length (not of length 5)");
#endif
      }
      // TODO check option, rssi bytes    
    }   //end     if (xbee.getResponse().getApiId() == RX_16_RESPONSE)

    else
    {
      // not something we were expecting
      Serial.println("Received a response that is NOT RX_16_RESPONSE");
      Serial.print("response is: ");
      Serial.println(xbee.getResponse().getApiId());
      Serial.print("HEX: ");
      Serial.println(xbee.getResponse().getApiId(), HEX);
      Serial.print("BIN: ");
      Serial.println(xbee.getResponse().getApiId(), BIN);
      Serial.print("DEC: ");
      Serial.println(xbee.getResponse().getApiId(), DEC);
      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE)
      {
        Serial.println("TX_STATUS_RESPONSE RECEIVED");
      }
    }
  } // end if (xbee.getResponse().isAvailable())
  else if (xbee.getResponse().isError())
  {
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  } 
}  // end void getResponses()

void calcLED_States()
{
#ifdef LOG_SERIAL
  Serial.println();
  Serial.print("at start of void calcLED_States() function, toggleReq[i]:");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    Serial.print(" ");
    Serial.print(toggleReq[i]);
  }
#endif

  for (int i = 0; i < numCounterMeasures; i++)
  {
    // (cmPB_State_Now[i] && !cmPB_State_Prev[i] || !cmPB_State_Now[i] && cmPB_State_Prev[i]) forms the exclusive or of cmPB_State_Now[i] and cmPB_State_Prev[i] 
    //(cmPB_State_Now[i] xor cmPB_State_Prev[i]) && cmPB_State_Now[i] creates pulse in loop when cmPB_State_Now[i] first goes high
    int cmPB_Pulse = ((cmPB_State_Now[i] && !cmPB_State_Prev[i] || !cmPB_State_Now[i] && cmPB_State_Prev[i]) && cmPB_State_Now[i]);  //generate pulse when cmPB_State_Now[i] first goes high
    bool_cmPB_State_Now[i] = false;
    bool_cmPB_State_Prev[i] = false;
    if (cmPB_State_Now[i] ==1) bool_cmPB_State_Now[i] = true;
    if (cmPB_State_Prev[i] ==1) bool_cmPB_State_Prev[i] = true;
    boolean bool_cmPB_Pulse = ((bool_cmPB_State_Now[i] && !bool_cmPB_State_Prev[i] || !bool_cmPB_State_Now[i] && bool_cmPB_State_Prev[i]) && bool_cmPB_State_Now[i]);
#ifdef LOG_SERIAL
    Serial.println();
    Serial.println("***in void calcLED_States()***");
    Serial.print("bool_cmPB_State_Now[i]: ");
    Serial.println(bool_cmPB_State_Now[i]);
    Serial.print("bool_cmPB_State_Prev[i]: ");
    Serial.println(bool_cmPB_State_Prev[i]);
    Serial.print("bool_cmPB_Pulse: ");
    Serial.println(bool_cmPB_Pulse);
    Serial.println("in void calcLED_States()");
    Serial.print(i);
    Serial.print(" ");
    Serial.print("cmPB_Pulse: ");
    Serial.print(cmPB_Pulse);
    Serial.print(" ");
    Serial.print("cmPB_State_Now/_Prev: ");
    Serial.print(cmPB_State_Now[i]);
    Serial.print("/");
    Serial.println(cmPB_State_Prev[i]);
#endif
    if (cmPB_Pulse || toggleReq[i])
    {
      cmLED_State_Now[i] = !cmLED_State_Now[i];
      //#ifdef LOG_SERIAL
      Serial.println("in void calcLED_States()");
      Serial.print("cmLED_State_Now[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(cmLED_State_Now[i]);
      //#endif
    } 
  }
}

void readAnalogs()
{
  //For testing purposes:
  //analog values for throttle position, voltage, current, 
  //& controller temperature are either read or simulated.
  //Motor temperature is read from Arduino pin A0.
  //
  motorTemperatureValueADC = analogRead(motorTemperaturePin);

#ifdef LOG_SERIAL
  Serial.print("motorTemperatureValueADC: ");
  Serial.print(motorTemperatureValueADC);
  Serial.println(" in readAnalogs() function");
#endif

  boolean simulateAnalogs = false;
  simulateAnalogs = true;  //comment this line out if using live data

  if (simulateAnalogs)
  {
    //    throttlePosition = 10;
    throttlePosition = random(0,110);
    //    voltage = 25;
    voltage = random(0,50);
    //    current = 100;
    current = random(0,330);
    //    controllerTemperature = 75;
    controllerTemperature = random(0,110);

  }
  else
  {
  }
}

void nullPayload()
{
  for (int i = 0; i < sizeof(payload); i++)
  {
    payload[i] = char(0);

  }
}

void setup()
{
  //setup up outputs
  for (int i = 0; i < numCounterMeasures; i++)
  {
    pinMode(cmLED_Pins[i], OUTPUT);
  }
  for (int i = 0; i < 3; i++)
  {
    pinMode(cm_Pins[i], OUTPUT);
  }
  pinMode(PIT_IN_LED, OUTPUT);
  //setup up inputs
  for (int i = 0; i < numCounterMeasures; i++)
  {
    pinMode(cmPB_Pins[i], INPUT);
  }

  xbee.begin(9600);
  for (int i = 0; i < sizeof(payload); i++)
  {
    Serial.print("payload[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(payload[i],DEC);
  }
}

void loop()
{
  // start transmitting after a startup delay.  Note: this will rollover to 0 eventually so not best way to handle
  if (millis() - start > 15000) {
    // break down 10-bit reading into two bytes and place in payload
    //pin5 = analogRead(5);
    //    payload[1] = pin5 >> 8 & 0xff;
    //    payload[2] = pin5 & 0xff;

    //    send_tx();    //    xbee.send(tx);

    // flash TX indicator
  }

  //Reset toggleReq to false at start of loop
  for (int i = 0; i < numCounterMeasures; i++)
  {
    toggleReq[i] = false;
  }

  //Read state of field (CAR).
  //this includes:
  //H/W PBs states
  //H/W LED states
  //controller analog values

  //*************
  //Motor temp (Note:  this is broadcast by XBee)
  //20140616 REV7_0 changes the way motor temp is handled...
  //instead of being broadcast by XBee, it is being placed on Arduino A0.
  //The motor temp ADC counts will be read by Arduino using analogRead, 
  //the corresponding temperature in degrees C will be determined,
  //and this temperature will be sent to the PIT using the send_tx function. 
  //*************

  //Read H/W PBs states
  // and set cmPB_State_Now[i] accordingly
#ifdef LOG_SERIAL
  Serial.println();
  Serial.print("reading H/W PBs states in loop()");
#endif
  for (int i = 0; i < numCounterMeasures; i++)
  {
    int *ptr_cmPB_Pins;
    int *ptrNumber;
    int value;
    //    value = digitalRead(cmPB_Pins[i]);
    ptr_cmPB_Pins = &cmPB_Pins[i];
    //    cmPB_State_Now[i] = digitalRead(*ptr_cmPB_Pins);
    cmPB_State_Now[i] = digitalRead(cmPB_Pins[i]);
  }

  //#ifdef LOG_SERIAL
  Serial.println();
  Serial.println("***in void loop()***");
  Serial.print("cmPB_State_Now: ");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    Serial.print(" ");
    Serial.print(cmPB_State_Now[i]);
  }
  //#endif

#ifdef LOG_SERIAL
  Serial.println();
  Serial.print("cmPB_State_Prev:");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    Serial.print(" ");
    Serial.print(cmPB_State_Prev[i]);
  }
#endif

  //Read H/W LED states
  // and set cmLED_State_Now[i] accordingly
#ifdef LOG_SERIAL
  Serial.println();
  Serial.print("reading H/W LED states in loop()");
#endif
  for (int i = 0; i < numCounterMeasures; i++)
  {
    int *ptr_cmLED_Pins;
    int value;
    //    value = digitalRead(cmLED_Pins[i]);
    ptr_cmLED_Pins = &cmLED_Pins[i];
    //    cmLED_State_Now[i] = digitalRead(*ptr_cmLED_Pins);
    cmLED_State_Now[i] = digitalRead(cmLED_Pins[i]);
  }

  //#ifdef LOG_SERIAL
  Serial.println();
  Serial.println("***in void loop()***");
  Serial.print("cmLED_State_Now: ");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    Serial.print(" ");
    Serial.print(cmLED_State_Now[i]);
  }
  //#endif

  //#ifdef LOG_SERIAL
  Serial.println();
  Serial.println("***in void loop()***");
  Serial.print("cmLED_State_Prev:");
  for (int i = 0; i < numCounterMeasures; i++)
  {
    Serial.print(" ");
    Serial.print(cmLED_State_Prev[i]);
  }
  Serial.println();
  //#endif

  //Read controller analog values
  readAnalogs();  

  //Read toggle requests for cmLEDs from PIT
  //  getResponses();
  checkForRcvdPacket();

  //Determine states of cm1 thru 4 LEDs
  calcLED_States();

  //Update H/W LED states
  //    #ifdef LOG_SERIAL
  Serial.println("***in void loop()***Update H/W LED states***");
  //    #endif

  for (int i = 0; i < numCounterMeasures; i++)
  {
    digitalWrite(cmLED_Pins[i], cmLED_State_Now[i]);
    //    #ifdef LOG_SERIAL
    Serial.print("writing to cmLED_Pins[");
    Serial.print(i);
    Serial.print("] the value ");
    Serial.println(cmLED_State_Now[i]);
    //      #endif
  }

  //Update XBee with H/W LED states.
  //Note:  updating every loop cycle will ensure that S/W LED states in processing
  //program will agree with H/W LED states.
  payload[0] = 'C';
  payload[1] = 'M';
  pitAlivePulse = ((pitAliveNow && !pitAlivePrev) || (!pitAliveNow && pitAlivePrev)) && pitAliveNow;

#ifdef LOG_SERIAL
  Serial.print("pitAlivePulse: ");
  Serial.println(pitAlivePulse);
#endif

  for (int i = 0; i < numCounterMeasures; i++)
  {
    //    if (cmLED_State_Now[i] != cmLED_State_Prev[i])  //update XBee with H/W LED states when they toggle
    //sending cmLED_State_Now[i] each loop cycle overloads PIT processing program so that s/w toggle requests do not get processed.
    //Therefore, try sending only when pitAlivePulse is true.  This will occur for only 1 looptime every 
    if ((cmLED_State_Now[i] != cmLED_State_Prev[i]) || pitAliveMsgRcvd)  //update XBee with H/W LED states when they toggle
    {
      payload[2] = i + 49;
      payload[3] = 'L';
      payload[4] = 'O';
      if (cmLED_State_Now[i] == 1)
      {
        payload[3] = 'H';
        payload[4] = 'I';
      }
      send_tx();
    }
  }
  pitAliveMsgRcvd = false;
  //Transmit throttle position, voltage, current once every loopsUntilXmit loops
  if (loopCounter > loopsUntilXmit)
  {
    loopCounter = 0;  //re-initialize loopCounter
    //Update XBee with controller analog values
    //send motorTemperatureValueADC
    //Linearize motor temperature (degC) with slope & Intercept.
    //Convert degC to degF.
    nullPayload();
    float slope = -0.155611606;
    float y_intercept = 180.5743428;
    pLoad = (motorTemperatureValueADC * slope + y_intercept) * 9.0 / 5.0 + 32.0;

    //    #ifdef LOG_SERIAL
    Serial.print("motorTemperatureValueADC / pLoad: ");
    Serial.print(motorTemperatureValueADC);
    Serial.print(" / ");
    Serial.println(pLoad);
    //    #endif

    payload[0] = 'M';
    //    payload[3] = motorTemperatureValueADC >> 8 & 0xff;
    //    payload[4] = motorTemperatureValueADC & 0xff;
    convertCharDataToDecimal();

#ifdef LOG_SERIAL
    Serial.print("motorTemperatureValueADC/payload[3]/[4]: ");
    Serial.print(motorTemperatureValueADC);
    Serial.print("\t");
    Serial.print(payload[3]);
    Serial.print("\t");
    Serial.println(payload[4]);
#endif

    send_tx();

    //send throttle position
    nullPayload();
    pLoad = throttlePosition;
    //    pLoad = random(0,110);
    payload[0] = 'P';
    convertCharDataToDecimal();
    send_tx();
    //send voltage
    nullPayload();
    pLoad = voltage;
    //    pLoad = random(0,50);
    payload[0] = 'V';
    convertCharDataToDecimal();
    //    payload[3] = '2';
    //    payload[4] = '5';
    send_tx();
    //send current
    nullPayload();
    pLoad = current;
    //    pLoad = random (0,330);
    payload[0] = 'I';
    convertCharDataToDecimal();
    send_tx();
    //send controller temperature
    nullPayload();
    pLoad = controllerTemperature;
    //    pLoad = random (0,110);
    payload[0] = 'T';
    convertCharDataToDecimal();
    send_tx();
  }
  else
  {
    loopCounter++;
  }
  //Remember H/W PBs states for next loop
#ifdef LOG_SERIAL
  Serial.println("saving cmPB_State_Now for next loop");
#endif

  for (int i = 0; i < numCounterMeasures; i++)
  {
#ifdef LOG_SERIAL
    Serial.print("cmPB_State_Now[");
    Serial.print(i);
    Serial.print("]");
    Serial.println(cmPB_State_Now[i]);
#endif
    cmPB_State_Prev[i] = cmPB_State_Now[i];
  }

  //Remember H/W LED states for next loop
  for (int i = 0; i < numCounterMeasures; i++)
  {
    cmLED_State_Prev[i] = cmLED_State_Now[i];
  }

  //Remember states for next loop
  pitAlivePrev = pitAliveNow;

  delay(2000);
}  //end of loop()


















































