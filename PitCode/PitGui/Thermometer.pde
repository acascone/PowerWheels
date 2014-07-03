class Thermometer extends Indicator {
  // Variables are inherited from the parent.
  // We could also add variables unique to the Thermometer class if we so desire

  Thermometer(String _address, int _sizeX, int _sizeY, int _posX, int _posY) {
    // If the parent constructor takes arguments then super() needs to pass in those arguments.
    super(_address, _sizeX, _sizeY, _posX, _posY);
  }

  Thermometer(String _address, int _sizeX, int _sizeY, int _posX, int _posY, String _label, String _units, int _minDisplay, int _maxDisplay) { // initialize thermometer object
    super(_address, _sizeX, _sizeY, _posX, _posY, _label, _units, _minDisplay, _maxDisplay);
  }
}

