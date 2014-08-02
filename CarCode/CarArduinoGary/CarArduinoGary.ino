#include <XBee.h>
#include <Alltrax.h>

//#define LOG_SERIAL    //log to serial device when uncommented

// For testing purposes: analog values for throttle position, voltage,
// current, and controller temperature are either read or simulated.
const boolean simulateAnalogs = false;
const uint16_t PIT_ADDRESS = 0x0005;

const float slope = -0.155611606;      // for temperature conversion
const float y_intercept = 180.5743428; // for temperature conversion

const int numCounterMeasures = 4;
const int loopsUntilXmit     = 5; //transmit telemetry once every 5 loops
const unsigned int pitMsgTimeout = 2000;

// Arduino Pin Assignments
const int cmLED_Pins[numCounterMeasures] = {13, 12, 5, 7}; // pins for counter measure indicator LEDs
const int cmPB_Pins[numCounterMeasures] = {4, 6, 8, 9};    // pins for counter measure arming PBs
const int cm_Pins[numCounterMeasures] = {3, 10, 11, A5};   // pins for counter measure firing
const int PIT_IN_LED = 2;                                  // PIT IN LED is on Arduino pin 2
const int motorTemperaturePin = A0;                        // connects to motor thermistor

// XBee Pin Assignemnts
const int XBeeFirePin = 3;
const int XBeePitPin = 4;

// Global Memory
XBee xbee;
Alltrax controller;

boolean toggleReq[numCounterMeasures] = {false, false, false, false};

boolean cmPB_State_Now[numCounterMeasures] = {0, 0, 0, 0};   // current state of counter measure PBs
boolean cmPB_State_Prev[numCounterMeasures] = {0, 0, 0, 0};  // previous state of counter measure PBs
boolean cmLED_State_Now[numCounterMeasures] = {0, 0, 0, 0};  // current state of counter measure LEDs
boolean cmLED_State_Prev[numCounterMeasures] = {0, 0, 0, 0}; // previous state of counter measure LEDs

unsigned long pitLastMsgTime = 0;
boolean pitAlive = false;
boolean pitAliveMsgRcvd = false;

uint8_t payload[] = {0, 0, 0, 0, 0}; // allocate five bytes for to hold a payload

int throttlePosition      = 0;
int voltage               = 0;
int current               = 0;
int controllerTemperature = 0;
int motorTemperature      = 0;

void setup()
{
  for (int i = 0; i < numCounterMeasures; i++)
  {
    pinMode(cmLED_Pins[i], OUTPUT);
    pinMode(cm_Pins[i], OUTPUT);
    pinMode(cmPB_Pins[i], INPUT);
  }
  
  pinMode(PIT_IN_LED, OUTPUT);
  
  controller.begin(&Serial1);
  
  Serial2.begin(57600);
  xbee.setSerial(Serial2);
}

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

void handle_RX_16_RESPONSE(Rx16Response &rx16)
{
  //Valid RX_16_RESPONSE should have rx16.getDataLength() = 5
  //and rx16.getData(4) will hold either an "E" (ALIVE)
  //or the number 1 through 4 if PIT sends a toggle request (TGCM1, TGCM2, TGCM3, TGCM4).
  if (rx16.getDataLength() == 5)
  {
    pitLastMsgTime = millis();  // timestamp of last message received
    
    if (rx16.getData(4) == 'E')
    {
      pitAliveMsgRcvd = true;  //when true, a new pit ALIVE msg has been received from PIT
    }
    else if (rx16.getData(4) >= '1' && rx16.getData(4) <= '4')
    {
      toggleReq[(char)rx16.getData(4) - '1'] = true;
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
  TxStatusResponse txStatus;
  
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
    {
      Rx16Response rx16;
      response.getRx16Response(rx16);
      handle_RX_16_RESPONSE(rx16);
      break;
    }
    case RX_16_IO_RESPONSE:
    {
      Rx16IoSampleResponse ioSample;
      response.getRx16IoSampleResponse(ioSample);
      
      if (ioSample.isDigitalEnabled(XBeePitPin))
      {
        digitalWrite(PIT_IN_LED, HIGH);
      }
      else
      {
        digitalWrite(PIT_IN_LED, LOW);
      }
      
      // This logic seems inverted, but may be intentional.
      if (ioSample.isDigitalEnabled(XBeeFirePin))
      {
        // disable counter measure devices
        for (int i = 0; i < numCounterMeasures; i++)
        {
          digitalWrite(cm_Pins[i], LOW);
        }
      }
      else
      {
        // activate counter measures if armed
        for (int i = 0; i < numCounterMeasures; i++)
        {
          // set pin high if armed
          digitalWrite(cm_Pins[i], cmLED_State_Now[i]);
          // clear state after firing countermeasure
          cmLED_State_Now[i] = 0;
          // set new led stat
          digitalWrite(cmLED_Pins[i], cmLED_State_Now[i]);
        }
      }
      break;
    }
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

boolean send_tx(uint16_t addr, uint8_t *data, uint8_t len)
{
  Tx16Request tx = Tx16Request(addr, data, len);
  TxStatusResponse txStatus;
  boolean ret = false;
  
  xbee.send(tx);

  // after sending a tx request, we expect a status response
  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      
      // get the delivery status, the fifth byte
      if (txStatus.getStatus() == SUCCESS)
      {
        ret = true;
      }
      else
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
  
  return ret;
}

void calcLED_States()
{
  for (int i = 0; i < numCounterMeasures; i++)
  {
    bool cmPB_Pulse = (((cmPB_State_Now[i] && !cmPB_State_Prev[i]) || 
                       (!cmPB_State_Now[i] && cmPB_State_Prev[i])) && 
                       cmPB_State_Now[i]);  //generate pulse when cmPB_State_Now[i] first goes high
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
  
  if (simulateAnalogs)
  {
    throttlePosition = random(0,110);
    voltage = random(0,50);
    current = random(0,330);
    controllerTemperature = random(0,110);
  }
  else
  {
    throttlePosition = controller.readThrottlePercent();
    voltage = controller.readVoltage();
    current = controller.readCurrent();
    controllerTemperature = controller.readTemp();
  }
}

void loop()
{
  static int loopCounter = 0;
  
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

  for (int i = 0; i < numCounterMeasures; i++)
  {
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
      send_tx(PIT_ADDRESS, payload, sizeof(payload));
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
    send_tx(PIT_ADDRESS, payload, sizeof(payload));

    //send throttle position
    payload[0] = 'P';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,throttlePosition);
    send_tx(PIT_ADDRESS, payload, sizeof(payload));
    
    //send voltage
    payload[0] = 'V';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,voltage);
    send_tx(PIT_ADDRESS, payload, sizeof(payload));
    
    //send current
    payload[0] = 'I';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,current);
    send_tx(PIT_ADDRESS, payload, sizeof(payload));
    
    //send controller temperature
    payload[0] = 'T';
    convertCharDataToDecimal(payload+1,sizeof(payload)-1,controllerTemperature);
    send_tx(PIT_ADDRESS, payload, sizeof(payload));
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
  
  // TODO: set LED high with PIT is alive
  pitAlive = (millis() - pitLastMsgTime) < pitMsgTimeout;
  
  // this is not great...
  //delay(100);
}

