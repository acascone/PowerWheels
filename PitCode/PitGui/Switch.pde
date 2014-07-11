// defines the switch objects and their behaviors
boolean armed = false;
class Switch {
  ArrayList<Switch> switches;  //an arrayList for all the switches
  final int yPosition = 700;
  int switchNumber, posX, posY;
  boolean state = false; // current switch state
  PImage on, off;        // stores the pictures of the on and off switches
  String label;

  // initialize switch object:
  Switch(int _switchNumber) { 
    this(_switchNumber, "");
  }
  // initialize switch object:
  Switch(int _switchNumber, String _label) { 
    on = loadImage("on.jpg");
    off = loadImage("off.jpg");
    switchNumber = _switchNumber;
    posX = switchNumber * (on.width+ 40) + 40;
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
    } else
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
    if (mouseX >=posX && mouseY >= posY && mouseX <=posX+on.width && mouseY <= posY+on.height) 
    {
      //     println("clicked on " + address);
      //      state = !state; // change the state of the switch if it was clicked
      int switchNum = switchCounter;
      toggleReq[switchNum] = true;
    }
  }  //end toggleState
} //end of switch class

