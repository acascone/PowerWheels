class AnalogMeter extends Indicator {
  // Variables are inherited from the parent.
  // We could also add variables unique to the AnalogMeter class if we so desire

  AnalogMeter(String _address, int _sizeX, int _sizeY, int _posX, int _posY) {
    // If the parent constructor takes arguments then super() needs to pass in those arguments.
    super(_address, _sizeX, _sizeY, _posX, _posY);
  }
  AnalogMeter(String _address, int _sizeX, int _sizeY, int _posX, int _posY, String _label, String _units, int _minDisplay, int _maxDisplay) {
    // If the parent constructor takes arguments then super() needs to pass in those arguments.
    super(_address, _sizeX, _sizeY, _posX, _posY, _label, _units, _minDisplay, _maxDisplay);
  }

  // draw thermometer on screen
  void render()
  {
    noStroke(); // remove shape edges
    int stemSize = 30; // stem augments fixed red bulb 
    
    // to help separate it from moving mercury
    // limit display to range
    float displayTemp = round( temp);
    if (temp > maxDisplay)
    {
      displayTemp = maxDisplay + 1;
    }
    if ((int)temp < minDisplay)
    {
      displayTemp = minDisplay-1;
    }
    
    // size for variable red area:
    float mercury = (displayTemp-minDisplay) / (maxDisplay-minDisplay); 
    
    // draw edges of objects in black
    fill(0); 
    rect(posX-3, posY-3, sizeX+5, sizeY+5); 
    
    // draw grey mercury background
    fill(64); 
    rect(posX, posY, sizeX, sizeY);
    // draw red areas
    fill(255, 16, 16);

    // draw mercury area:
    rect(posX, posY, sizeX * mercury, sizeY);

    // draw grey rect to hide mercury & create needle
    fill(64);
    
    if (temp > minDisplay)
    {
      rect(posX, posY, sizeX * mercury - 5, sizeY);
    }

    // show text
    textAlign(LEFT);
    fill(0);
    textSize(10);

    // show sensor address:
    text(label + " (" + units + ")", posX+sizeX/2 - 20, posY + sizeY + 4, 165, 40);

    // show units
    //    text("(" + units + ")", posX+sizeX/2, posY + sizeY + 25);

    // show maximum temperature: 
    text(maxDisplay + "", posX+sizeX+5, posY + sizeY + 4, 65, 40); 

    // show minimum temperature:
    text(minDisplay + "", posX - 15, posY + sizeY + 4, 65, 40); 

    // show temperature:
    text(round(temp), posX + sizeX * mercury, posY - 14);
  }
}

