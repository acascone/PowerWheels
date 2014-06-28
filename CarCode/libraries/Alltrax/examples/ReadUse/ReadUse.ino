#include <Alltrax.h> //Import the motor controller Library

//This is how to properly set up a HardwareSerial version of Alltrax
//Note there is a begin() function for the HardwareSerial version, make sure that gets called
Alltrax controller;

typedef struct {
  float voltage;
  float current;
  int16_t temperature;
  int16_t throttle;
} msg_data_s;

msg_data_s data;

// this is the header for an XBee TX64 Request
const uint8_t header[] = {0x7E,0x00,0x17,0x00,0x00,0x00,0x13,0xA2,0x00,0x40,0xA0,0xB1,0xD4,0x01};

void setup()
{
  controller.begin(&Serial1);
  //Alltrax setup is complete
  
  Serial.begin(9600);//sets up the arduino to communicate with the computer
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo and Micro only
  }
}

void loop(){
  // outgoing message buffer
  uint8_t buf[27];
  
  // read all the data from the motor controller
  data.voltage     = controller.readVoltage();
  data.temperature = controller.readTemp();
  data.current     = controller.readCurrent();
  data.throttle    = controller.readThrottle();
  
  // prepare the outgoing message buffer
  memcpy(buf,header,sizeof(header));
  memcpy(buf+sizeof(header),&data,sizeof(data));
  
  // calculate the checksum
  uint8_t cs = 0;
  for (int i = 3; i < sizeof(buf)-1; i++) cs += buf[i];
  cs = 0xFF - cs;
  
  // set the checksum byte in the message
  buf[26] = cs;
  
  // Print the raw byte of the message to the console for debug purposes
  for(int i = 0; i < sizeof(buf); i++) {
    //if (buf[i] < 0x10) Serial.print("0"); // add zero padding for readability
    //Serial.print(buf[i],HEX);
    Serial.write(buf[i]);
  }
  //Serial.print("\n");
  
  // Delay for readability
  delay(100);
}
