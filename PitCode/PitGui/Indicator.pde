// A class to describe a group of Shapes
// An ArrayList is used to manage the list of Shapes 

// defines the indicator objects
class Indicator {
  
  ArrayList<Indicator> indicators;  //an arrayList for all the indicators
  
  int sizeX, sizeY, posX, posY;
  int maxTemp = 200; // max of scale in degrees Fahrenheit
  int minTemp = 70; // min of scale in degress Fahrenheit
  float temp; // stores the temperature locally
  String address; // stores the address locally
  String label;
  String units;
  int minDisplay;
  int maxDisplay;

  Indicator(String _address, int _sizeX, int _sizeY, int _posX, int _posY) { // initialize thermometer object
    address = _address;
    sizeX = _sizeX;
    sizeY = _sizeY;
    posX = _posX;
    posY = _posY;
    indicators = new ArrayList<Indicator>();  //initialize the arraylist
  }
  Indicator(String _address, int _sizeX, int _sizeY, int _posX, int _posY, String _label, String _units, int _minDisplay, int _maxDisplay) { // initialize thermometer object
    address = _address;
    sizeX = _sizeX;
    sizeY = _sizeY;
    posX = _posX;
    posY = _posY;
    label = _label;
    units = _units;
    minDisplay = _minDisplay;
    maxDisplay = _maxDisplay;
    indicators = new ArrayList<Indicator>();  //initialize the arraylist
  }

  void render() { // draw thermometer on screen
//  println("render called............");
    noStroke(); // remove shape edges
    ellipseMode(CENTER); // center bulb
    float bulbSize = sizeX + (sizeX * 0.5); // determine bulb size
    int stemSize = 30; // stem augments fixed red bulb 
    // to help separate it from moving mercury
    // limit display to range
    float displayTemp = round( temp);
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
      displayTemp = minDisplay;
    }
    // size for variable red area:
//    float mercury = ( 1 - ( (displayTemp-minTemp) / (maxTemp-minTemp) )); 
    float mercury = ( 1 - ( (displayTemp-minDisplay) / (maxDisplay-minDisplay) )); 
    // draw edges of objects in black
    fill(0); 
    rect(posX-3,posY-3,sizeX+5,sizeY+5); 
    ellipse(posX+sizeX/2,posY+sizeY+stemSize, bulbSize+4,bulbSize+4);
    rect(posX-3, posY+sizeY, sizeX+5,stemSize+5);
    // draw grey mercury background
    fill(64); 
    rect(posX,posY,sizeX,sizeY);
    // draw red areas
    fill(255,16,16,1000);

    // draw mercury area:
    rect(posX,posY+(sizeY * mercury), 
    sizeX, sizeY-(sizeY * mercury));

    // draw stem area:
    rect(posX, posY+sizeY, sizeX,stemSize); 

    // draw red bulb:
    ellipse(posX+sizeX/2,posY+sizeY + stemSize, bulbSize,bulbSize); 

    // show text
    textAlign(LEFT);
//    textAlign(CENTER);
    fill(0);
    textSize(10);

    // show sensor address:
//    text(address, posX-10, posY + sizeY + bulbSize + stemSize + 4, 65, 40);
    text(label, posX-10, posY + sizeY + bulbSize + stemSize + 4, 75, 40);

    // show maximum temperature: 
//    text(maxTemp + "˚F", posX+sizeX + 5, posY); 
    text(maxDisplay + "˚F", posX+sizeX + 5, posY); 

    // show minimum temperature:
//    text(minTemp + "˚F", posX+sizeX + 5, posY + sizeY); 
    text(minDisplay + "˚F", posX+sizeX + 5, posY + sizeY); 

    // show temperature:
    text(round(temp) + " ˚F", posX+2,posY+(sizeY * mercury+ 14));
  }
}

