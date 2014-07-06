// defines the switch objects and their behaviors
boolean armed = false;
class Switch {
  ArrayList<Switch> switches;  //an arrayList for all the switches
  final int yPosition = 700;
    int switchNumber, posX, posY;
  boolean state = false; // current switch state
  //  XBeeAddress64 addr64;  // stores the raw address locally
  //  String address;        // stores the formatted address locally
  PImage on, off;        // stores the pictures of the on and off switches
  int counter = 0;
  String label;

  // initialize switch object:
  Switch(int _switchNumber) { 
    on = loadImage("on.jpg");
    off = loadImage("off.jpg");
    switchNumber = _switchNumber;
    posX = switchNumber * (on.width+ 40) + 40;
    //    posY = 50;
    posY = yPosition;
    switches = new ArrayList<Switch>();  //initialize the arraylist
  }
  // initialize switch object:
  Switch(int _switchNumber, String _label) { 
    on = loadImage("on.jpg");
    off = loadImage("off.jpg");
    switchNumber = _switchNumber;
    posX = switchNumber * (on.width+ 40) + 40;
    //    posY = 50;
    posY = yPosition;
    label = _label;
    switches = new ArrayList<Switch>();  //initialize the arraylist
  }

  void render() { // draw switch on screen
    noStroke(); // remove shape edges
    //label switch
    if (state)
    {
      image(on, posX, posY); // if the switch is on, draw the on image
    }
    else
    {
      image(off, posX, posY);     // otherwise if the switch is off,
    } 
    // draw the off image
    // show text
    textAlign(CENTER);
    fill(0);
    textSize(10);
    // show actuator address
    //    text(address, posX+on.width/2, posY + on.height + 10);
    // show on/off state
    String stateText = "DISARMED";
    //    fill (255, 0, 0);
    if (state) {
      stateText = "ARMED";
      fill(0, 127, 0);
    }
    text(stateText, posX + on.width/2, posY-8);
    fill(0);
    text(label, posX + on.width/2, posY-20);
  }

  /*
  // checks the remote actuator node to see if its on or off currently
   void getState() {
   try {
   println("node to query: " + addr64);
   
   // query actuator device (pin 20) D0 (Digital output high = 5, low = 4)
   // ask for the state of the D0 pin: 
   ZNetRemoteAtRequest request=  new ZNetRemoteAtRequest(addr64, "D0"); 
   
   // parse the response with a 10s timeout 
   ZNetRemoteAtResponse response = (ZNetRemoteAtResponse)
   xbee.sendSynchronous(request, 10000); 
   
   if (response.isOk()) {
   
   // get the state of the actuator from the response
   int[] responseArray = response.getValue();
   int responseInt = (int) (responseArray[0]);
   
   // if the response is good then store the on/off state: 
   if(responseInt == 4|| responseInt == 5) { 
   // state of pin is 4 for off and 5 for on:
   state = boolean( responseInt - 4);  
if (LOG_SERIAL)
{
   println("successfully got state " + state + " for pin 20 (D0)");
}
   }
   else {  
   // if the current state is unsupported (like an analog input)
   // then print an error to the console
   println("unsupported setting " + responseInt + " on pin 20 (D0)");
   }
   } 
   // if there's an error in the response, print that to the 
   // console and throw an exception
   else {
   throw new RuntimeException("failed to get state for pin 20. " +
   " status is " + response.getStatus());
   }
   } 
   // print an error if there's a timeout waiting for the response
   catch (XBeeTimeoutException e) {
   println("XBee request timed out. Check remote's configuaration, " +
   " range and power");
   } 
   // print an error message for any other errors the occur
   catch (Exception e) {
   println("2.  unexpected error: " + e + "  Error text: " + e.getMessage());
   }
   }
   */

  void getState()
  {
  }
  // this function is called to check for a mouse click
  // on the switch object, and toggle the switch accordingly
  // it is called by the MousePressed() function so we already
  // know that the user just clicked the mouse somewhere
  // Send transmission to CAR arduino requesting that selected
  // switch be toggled.
  // When PIT gets transmission from CAR with state of switch,
  // set switch graphic to match.
  void toggleState()
  {
    // check to see if the user clicked the mouse on this particular switch
    if (mouseX >=posX && mouseY >= posY && 
      mouseX <=posX+on.width && mouseY <= posY+on.height) 
    {
      //     println("clicked on " + address);
      //      state = !state; // change the state of the switch if it was clicked
      int switchNum = switchCounter;
      toggleReq[switchNum] = true;
/*
//if (LOG_SERIAL)
//{
      println("Change to a Switch State " + char(switchNum+49) + " requested");
//}
      // create a unicast packet to be delivered to remote radio with 16-bit address: 0002, with payload "ALIVE"
      TxRequest16 request = new TxRequest16(new XBeeAddress16(0x00, 0x02), new int[] {
        'T', 'G', 'C', 'M', char(switchNum+49)
      }
      );
      println("calling send_TxTequest16 from toggleState");
      send_TxRequest16(request);
        checkForReceivedPacket();
  processReceivedData();
        checkForReceivedPacket();
  processReceivedData();

      //TODO check if I need 2 getResponse1() function calls...or if 1 call will work
            getResponse1();
            getResponse1();
*/            
//            state = false;
//            if (response.getApiId() == ApiId.RX_16_RESPONSE)
//            {
//              if (data[0] == 'C' & data[2] == '1' & data[3] == 'H')
//              {
//                state = true;
//                println("changing state of switch: " + switchCounter + " to..." + state);
//              }
//            }
          }  //end if (mouseX >=posX && mouseY >= posY && mouseX <=posX+on.width && mouseY <= posY+on.height)
    }  //end toggleState

    void send_Tx64_request(int Arm_on[])
    {
      for (int i = 0; i < Arm_on.length; i++)
      {
        Tx64_request_cmd[i] = Arm_on[i];
if (LOG_SERIAL)
{
  print(hex(Tx64_request_cmd[i], 2));
        print("  ");
}
        //Transmit RemoteAT ON request
        myPort.write(Tx64_request_cmd[i]);
      }
if (LOG_SERIAL)
{
      println("Tx64_request_cmd has been sent ");
}
    }
  } //end of switch class

