class Light {
  ArrayList<Light> lights;  //an arrayList for all the lights

    // Variables are inherited from the parent.
  // We could also add variables unique to the Light class if we so desire
  int sizeX, sizeY, posX, posY;
  String label; // stores the label locally
  int status;  //  stores monitored/displayed variable status locally

  Light(String _label, int _sizeX, int _sizeY, int _posX, int _posY, int _status) {
    //    // If the parent constructor takes arguments then super() needs to pass in those arguments.
    //    super( _label, _sizeX, _sizeY, _posX, _posY);
    label = _label;
    sizeX = _sizeX;
    sizeY = _sizeY;
    posX = _posX;
    posY = _posY;
    status = _status;
    lights = new ArrayList<Light>();  //initialize the arraylist
  }


  void render() { // draw switch on screen
    noStroke(); // remove shape edges
    //label switch
    if (status == 1)
    {
      fill(255, 0, 0);
      rect(posX, posY, sizeX, sizeY);
    }
    else
    {
      fill(0, 255, 0);
      rect(posX, posY, sizeX, sizeY);
    } 
    // draw the off image
    // show text
    textAlign(CENTER);
    fill(0);
    textSize(10);
    // show actuator label
    //    text(label, posX+on.width/2, posY + on.height + 10);
//    text(label, posX+sizeX/2, posY + sizeY + 10);
    text(label, posX+sizeX/2, posY-20);
    // show on/off state
    String stateText = "GOOD";
    fill (0,255,0);
    if (status == 1) {
      stateText = "ERROR";
      fill(255,0,0);
    }
    //    text(stateText, posX + on.width/2, posY-8);
//    text(stateText, posX, posY-8);
    text(stateText, posX+sizeX/2, posY + sizeY + 10);
    fill(0);
  }
}

