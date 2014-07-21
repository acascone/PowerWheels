/*
  *
  *20140721  GEM  Menu for PITGui
  *Provides means to connect to PIT XBee using either a Mac or Windows machine thru Ports tab.
  *Also provides capability to enable/disable logging thru Connectivity tab.
  *  
*/
import java.awt.*;
import java.awt.event.*;


MenuBar myMenu;
Menu topButton, secondButton;
MenuItem item1, item2, item3, item4, item5, item6;

myMenuListener menuListen;

void setup() {
  size(400, 200);
  //this doesn't demonstrate best coding practice, just a simple method
  //create the MenuBar Object
  menuListen = new myMenuListener();
  myMenu = new MenuBar();

  //create the top level button
  topButton = new Menu("Connections");
  secondButton = new Menu("Ports");

  //create all the Menu Items and add the menuListener to check their state.
  item1 = new MenuItem("Online");
  item1.addActionListener(menuListen);
  item2 = new MenuItem("Offline");
  item2.addActionListener(menuListen);
  item3 = new MenuItem("Disconnect");
  item3.enable(false);
  item3.addActionListener(menuListen);
  item4 = new MenuItem("Windows");
  item4.addActionListener(menuListen);
  item5 = new MenuItem("Mac");
  item5.addActionListener(menuListen);
  item6 = new MenuItem("Close");
  item6.addActionListener(menuListen);
  item6.enable(false);

  //add the items to the top level Button
  topButton.add(item1);
  topButton.add(item2);
  topButton.add(item3);
  //add the items to the second Button
  secondButton.add(item4);
  secondButton.add(item5);
  secondButton.add(item6);

  //add the button to the menu
  myMenu.add(topButton);

  //add second button to the menu
  myMenu.add(secondButton);
  //add the menu to the frame!
  frame.setMenuBar(myMenu);
}

void draw() {
  // get the current menu state
}

//this menuListener object is largely ripped off from http://java.sun.com/docs/books/tutorial/uiswing/examples/components/MenuDemoProject/src/components/MenuDemo.java

class myMenuListener implements ActionListener, ItemListener {

  myMenuListener() {
  }

  public void actionPerformed(ActionEvent e) {
    MenuItem source = (MenuItem)(e.getSource());
    String s = "Action event detected."
      + "    Event source: " + source.getLabel()
      + " (an instance of " + getClassName(source) + ")";
    println(s);

    if (source.getLabel().equals("Online")) {
      item2.enable(false);
      item3.enable(true);
    }
    else if (source.getLabel().equals("Offline")) {
      item1.enable(false);
      item3.enable(true);
    }
    else if (source.getLabel().equals("Disconnect")) {
      item1.enable(true);
      item2.enable(true);
      item3.enable(false);
    }
    else if (source.getLabel().equals("Windows")) {
      item5.enable(false);
      item6.enable(true);
    }
    else if (source.getLabel().equals("Mac")) {
      item4.enable(false);
      item6.enable(true);
    }
    else if (source.getLabel().equals("Close")) {
      item4.enable(true);
      item5.enable(true);
      item6.enable(false);
    }
  }

  public void itemStateChanged(ItemEvent e) {
    MenuItem source = (MenuItem)(e.getSource());
    String s = "Item event detected."
      + "    Event source: " + source.getLabel()
      + " (an instance of " + getClassName(source) + ")"
        + "    New state: "
        + ((e.getStateChange() == ItemEvent.SELECTED) ?
      "selected":"unselected");
    println(s);
  }
}

//gets the class name of an object
protected String getClassName(Object o) {
  String classString = o.getClass().getName();
  int dotIndex = classString.lastIndexOf(".");
  return classString.substring(dotIndex+1);
}

