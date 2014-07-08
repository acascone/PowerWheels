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

  void render() { // draw thermometer on screen
    //  println("render called............");
    noStroke(); // remove shape edges
    //    ellipseMode(CENTER); // center bulb
    //    float bulbSize = sizeX + (sizeX * 0.5); // determine bulb size
    int stemSize = 30; // stem augments fixed red bulb 
    // to help separate it from moving mercury
    // limit display to range
    float displayTemp = round( temp);
    //    println("temp: " + temp);
    //    if (temp > maxTemp) {
    //      displayTemp = maxTemp + 1;
    //    }
    //    if ((int)temp < minTemp) {
    //      displayTemp = minTemp;
    //    }
    if (temp > maxDisplay) {
      displayTemp = maxDisplay + 1;
    }
    if ((int)temp < minDisplay) {
      //      displayTemp = minDisplay;
      displayTemp = minDisplay-1;
    }
    // size for variable red area:
    //    float mercury = ( 1 - ( (displayTemp-minTemp) / (maxTemp-minTemp) )); 
    float mercury = (displayTemp-minDisplay) / (maxDisplay-minDisplay); 
    // draw edges of objects in black
    fill(0); 
    rect(posX-3, posY-3, sizeX+5, sizeY+5); 
    //    ellipse(posX+sizeX/2,posY+sizeY+stemSize, bulbSize+4,bulbSize+4);
    //   rect(posX-3, posY+sizeY, sizeX+5,stemSize+5);
    // draw grey mercury background
    fill(64); 
    rect(posX, posY, sizeX, sizeY);
    // draw red areas
    fill(255, 16, 16);

    // draw mercury area:
    //    println("posX, posY, sizeX, sizeY, mercury: " + posX + " " + posY + " " + sizeX + " " + sizeY + " " + mercury);
    rect(posX, posY, 
    sizeX * mercury, sizeY);

    // draw grey rect to hide mercury & create needle
    fill(64);
    //           if (displayValue > minDisplay) {
    if (temp > minDisplay) {
      rect(posX, posY, sizeX * mercury - 5, sizeY);
    }
    //          else {
    //                rect(posX, posY, sizeX * mercury + 5, sizeY);
    //
    //          } 


    // show text
    textAlign(LEFT);
    fill(0);
    textSize(10);

    // show sensor address:
    //    text(address, posX-10, posY + sizeY + bulbSize + stemSize + 4, 65, 40);
    //    println("label: " + label);
    text(label + " (" + units + ")", posX+sizeX/2 - 20, posY + sizeY + 4, 165, 40);

    // show units
    //    text("(" + units + ")", posX+sizeX/2, posY + sizeY + 25);


    // show maximum temperature: 
    //    text(maxTemp + "˚F", posX+sizeX + 5, posY);
    //    text(maxDisplay + units, posX+sizeX + 5, posY); 
    //    text(maxDisplay + units, posX+sizeX+5, posY + sizeY + 4, 65, 40); 
    text(maxDisplay + "", posX+sizeX+5, posY + sizeY + 4, 65, 40); 

    // show minimum temperature:
    //    text(minTemp + "˚F", posX - 25, posY); 
    //    text(minDisplay + units, posX - 25, posY); 
    //    text(minDisplay + units, posX - 25, posY + sizeY + 4, 65, 40); 
    text(minDisplay + "", posX - 15, posY + sizeY + 4, 65, 40); 

    // show temperature:
    //    text(round(temp) + " ˚F", posX+2,posY+(sizeY * mercury+ 14));
    //    text(round(temp) + " ˚F", posX + sizeX * mercury, posY - 14);
    //    text(round(temp) + units, posX + sizeX * mercury, posY - 14);
    text(round(temp), posX + sizeX * mercury, posY - 14);
  }
}

