#include <XBee.h>

//#define LOG_SERIAL    //log to serial device when uncommented

XBee xbee = XBee();
// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();
Rx16IoSampleResponse ioSample = Rx16IoSampleResponse();

//**********
//NOTE:  20140624:  So that XBee did not have to be re-programmed, we will
// use pin3 as FIRE and pin4 as PIT IN, so flip notes below
boolean pin3;  //stores status of PIT XBee AD03 (PIT IN)
boolean pin4;  //stores status of PIT XBee AD04 (FIRE)
//**********

uint8_t option = 0;
uint8_t data = 0;
const int numCounterMeasures = 4;
int cmLED_Pins[numCounterMeasures] = {13, 12, 5, 7}; // pins for counter measure LEDs
int cmPB_Pins[numCounterMeasures] = {4, 6, 8, 9};    //pins for counter measure arming PBs
const int PIT_IN_LED = 2;    //PIT IN LED is on Arduino pin 2
int cm_Pins[3] = {3, 10, 11};  //pins for counter measure PBs
boolean bool_cmPB_State_Now[numCounterMeasures] = {false, false, false, false};        // current state of counter measure PBs
boolean bool_cmPB_State_Prev[numCounterMeasures] = {false, false, false, false};        // previous state of counter measure PBs
int cmPB_State_Now[numCounterMeasures] = {0, 0, 0, 0};        // current state of counter measure PBs
int cmPB_State_Prev[numCounterMeasures] = {0, 0, 0, 0};        // previous state of counter measure PBs
int cmLED_State_Now[numCounterMeasures] = {0, 0, 0, 0};        // current state of counter measure LEDs
int cmLED_State_Prev[numCounterMeasures] = {0, 0, 0, 0};        // previous state of counter measure LEDs
boolean toggleReq[] = { false, false, false, false};
boolean aliveReq = false;
// pitUp variable indicates status of pit XBee/processing system...
// true = pit system up & working, false = not working
boolean pitAlive       = false;
boolean pitAliveNow    = false;
boolean pitAlivePrev   = false;
boolean pitAlivePulse  = false;
boolean pitAliveMsgRcvd = false;

// allocate five bytes for to hold a payload
uint8_t payload[] = {0, 0, 0, 0, 0};

// with Series 1 you can use either 16-bit or 64-bit addressing
// 16-bit addressing: Enter address of remote XBee, typically the coordinator
uint16_t PIT_ADDRESS = 0x0005;
Tx16Request tx = Tx16Request(PIT_ADDRESS, payload, sizeof(payload));
TxStatusResponse txStatus = TxStatusResponse();

int pin5 = 0;

const int motorTemperaturePin      = A0;  //A0 is pin that connects to motor thermistor
const float slope = -0.155611606;
const float y_intercept = 180.5743428;

// Variables will change:
int cm1LED_State          = LOW;      // variable for LED state for counter measure 1
long previousMillis       = 0;        // will store last time LED was updated
int cm1PB_State_Now       = 0;        // variable for current state of PB for counter measure 1 pushbutton
int cm1PB_State_Prev      = 0;        // variable for previous state of PB for counter measure 1 pushbutton
int pLoad                 = 0;        //variable used to convert character data to decimal
int throttlePosition      = 0;
int voltage               = 0;
int current               = 0;
int controllerTemperature = 0;
int motorTemperature      = 0;
int loopsUntilXmit        = 5;        //transmit throttle position, voltage, current, and controller temperature once every 5 loops
int loopCounter           = 0;        //Counts up to loopsUntilXmit, then resets to 0 when analog values are transmitted 

void convertCharDataToDecimal(uint8_t *p, int len, int value)
{
  for (int i = 0; i < len; i++)
  {
    int magnitude = pow(10, ((len-1) - i));
    p[i] = value / magnitude;
    value -= p[i] * magnitude;
    p[i] += '0';
  }
}

void handle_RX_16_RESPONSE()
{
  //Valid RX_16_RESPONSE should have rx16.getDataLength() = 5
  //and rx16.getData(4) will hold either an "E" (ALIVE)
  //or the number 1 through 4 if PIT sends a toggle request (TGCM1, TGCM2, TGCM3, TGCM4).
  if (rx16.getDataLength() == 5)
  {
    if (rx16.getData(4) == 'E')
    {
      pitAliveMsgRcvd = true;  //when true, a new pit ALIVE msg has been received from PIT
      pitAliveNow = true;  //when true, pit ALIVE msg has been received within the past "x" seconds; pit ALIVE msg is considered current
    }
    else if (rx16.getData(4) >= '1' && rx16.getData(4) <= '4')
    {
      toggleReq[(char)rx16.getData(4) - 49] = true;
    }
    else
    {
#ifdef LOG_SERIAL
      Serial.println("We received an RX_16_RESPONSE of length 5 BUT data in rx16.getData(4) was invalid");
#endif
    }
  }
  else    //else error
  {
#ifdef LOG_SERIAL
    Serial.println("We received a RX_16_RESPONSE BUT it was the wrong length (not of length 5)");
#endif
  }
}

void checkForRcvdPacket(XBeeResponse response)
{
  if (response.isAvailable())
  {
    switch (response.getApiId())
    {
    case  TX_STATUS_RESPONSE:
      //***************
      //NOTE:  We probably got here because ALIVE msg 
      //was received by CAR in the middle of a transmission by CAR.
      //***************

      //Even though the following line of code references a Series2 function, 
      //it is needed for Series1 XBees to get status of response 
      //which is tested for SUCCESS below.
      response.getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getStatus() != SUCCESS)
      {
        // the remote XBee did not receive our packet. is it powered on?
#ifdef LOG_SERIAL
        Serial.println("...BAD status...is remote XBee powered on?");
#endif
      }
      break;
    case  RX_16_RESPONSE:
      response.getRx16Response(rx16);
      handle_RX_16_RESPONSE();
      break;
    case RX_16_IO_RESPONSE:
      response.getRx16IoSampleResponse(ioSample);
      //      if (boolean pin3 = ioSample.isDigitalEnabled(3))
      pin3 = ioSample.isDigitalEnabled(3);
      pin4 = ioSample.isDigitalEnabled(4);
      
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
      {
        for (int i = 0; i < 3; i++)
        {
          digitalWrite(cm_Pins[i], cmLED_State_Now[i]);
        }
        for (int i = 0; i < 3; i++)
        {
          if (cmLED_State_Now[i] == 1)
          {
            cmLED_State_Now[i] = 0;
            digitalWrite(cmLED_Pins[i], cmLED_State_Now[i]);
          }
        }
      }
      break;
    default:
      // statements
#ifdef LOG_SERIAL
      Serial.println("default condition");
#endif
      break;
    }
  }
  else if (response.isError())
  {
#ifdef LOG_SERIAL
    Serial.println("if this msg prints, then there must have been a transmission error");
    Serial.print("response.getErrorCode(): ");
    Serial.println(response.getErrorCode());
#endif
  }
  else
  {
#ifdef LOG_SERIAL
    Serial.println("XBee port is NOT available");
#endif
  }
}

void send_tx()
{
  xbee.send(tx);

  // after sending a tx request, we expect a status response
  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      
      // get the delivery status, the fifth byte
      if (txStatus.getStatus() != SUCCESS)
      {
        // the remote XBee did not receive our packet. is it powered on?
#ifdef LOG_SERIAL
        Serial.println("bad status");
#endif
      }
    }
  }
  else if (xbee.getResponse().isError())
  {
    // or flash error led
#ifdef LOG_SERIAL
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
#endif
  } 
  else
  {
    // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
#ifdef LOG_SERIAL
    Serial.println("no timely TX Status Response...check that CAR serial port is plugged in");
#endif
  }
}

void calcLED_States()
{
  for (int i = 0; i < numCounterMeasures; i++)
  {
    bool cmPB_Pulse = (((cmPB_State_Now[i] && !cmPB_State_Prev[i]) || 
                       (!cmPB_State_Now[i] && cmPB_State_Prev[i])) && 
                       cmPB_State_Now[i]);  //generate pulse when cmPB_State_Now[i] first goes high
    bool_cmPB_State_Now[i] = false;
    bool_cmPB_State_Prev[i] = false;
    if (cmPB_State_Now[i] ==1) bool_cmPB_State_Now[i] = true;
    if (cmPB_State_Prev[i] ==1) bool_cmPB_State_Prev[i] = true;
    if (cmPB_Pulse || toggleReq[i])
    {
      cmLED_State_Now[i] = !cmLED_State_Now[i];
    } 
  }
}

void readAnalogs()
{
  int motorTemperatureValueADC = analogRead(motorTemperaturePin);

  //Linearize motor temperature (degC) with slope & Intercept.
  //Convert degC to degF.
  motorTemperature = (motorTemperatureValueADC * slope + y_intercept) * 9.0 / 5.0 + 32.0;
  
  //For testing purposes:
  //analog values for throttle position, voltage, current, 
  //& controller temperature are either read or simulated.
  boolean simulateAnalogs = false;
  simulateAnalogs = true;  //comment this line out if using live data

  if (simulateAnalogs)
  {
    throttlePosition = random(0,110);
    voltage = random(0,50);
    current = random(0,330);
    controllerTemperature = random(0,110);
  }
  else
  {
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
  
  Serial2.begin(9600);
  xbee.setSerial(Serial2);
}

void loop()
{
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

  //Read H/W PBs states
  // and set cmPB_State_Now[i] accordingly
  for (int i = 0; i < numCounterMeasures; i++)
  {
    cmPB_State_Now[i] = digitalRead(cmPB_Pins[i]);
  }

  //Read H/W LED states
  // and set cmLED_State_Now[i] accordingly
  for (int i = 0; i < numCounterMeasures; i++)
  {
    cmLED_State_Now[i] = digitalRead(cmLED_Pins[i]);
  }

  //Read controller analog values
  readAnalogs();  

  //Read toggle requests for cmLEDs from PIT
  //  getResponses();
  xbee.readPacket();
  checkForRcvdPacket(xbee.getResponse());

  //Determine states of cm1 thru 4 LEDs
  calcLED_States();

  //Update H/W LED states
  for (int i = 0; i < numCounterMeasures; i++)
  {
    digitalWrite(cmLED_Pins[i], cmLED_State_Now[i]);
  }

  //Update XBee with H/W LED states.
  //Note:  updating every loop cycle will ensure that S/W LED states in processing
  //program will agree with H/W LED states.
  payload[0] = 'C';
  payload[1] = 'M';
  pitAlivePulse = ((pitAliveNow && !pitAlivePrev) || (!pitAliveNow && pitAlivePrev)) && pitAliveNow;

  for (int i = 0; i < numCounterMeasures; i++)
  {
    //    if (cmLED_State_Now[i] != cmLED_State_Prev[i])  //update XBee with H/W LED states when they toggle
    //sending cmLED_State_Now[i] each loop cycle overloads PIT processing program so that s/w toggle requests do not get processed.
    //Therefore, try sending only when pitAlivePulse is true.  This will occur for only 1 looptime every 
    if ((cmLED_State_Now[i] != cmLED_State_Prev[i]) || pitAliveMsgRcvd)  //update XBee with H/W LED states when they toggle
    {
      payload[2] = i + '1';
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
    
    //send motorTemperature
    payload[0] = 'M';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,motorTemperature);
    send_tx();

    //send throttle position
    payload[0] = 'P';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,throttlePosition);
    send_tx();
    
    //send voltage
    payload[0] = 'V';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,voltage);
    send_tx();
    
    //send current
    payload[0] = 'I';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,current);
    send_tx();
    
    //send controller temperature
    payload[0] = 'T';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,controllerTemperature);
    send_tx();
  }
  else
  {
    loopCounter++;
  }
  
  //Remember H/W PBs states for next loop
  for (int i = 0; i < numCounterMeasures; i++)
  {
    cmPB_State_Prev[i] = cmPB_State_Now[i];
  }

  //Remember H/W LED states for next loop
  for (int i = 0; i < numCounterMeasures; i++)
  {
    cmLED_State_Prev[i] = cmLED_State_Now[i];
  }

  //Remember states for next loop
  pitAlivePrev = pitAliveNow;
  
  // this is not great...
  delay(100);
}

