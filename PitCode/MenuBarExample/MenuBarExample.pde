import java.awt.*;
import java.awt.event.*;

MenuBar myMenu;
Menu topButton;
MenuItem item1,item2,item3,item4,item5;

myMenuListener menuListen;

color bg = color(255);

void setup(){
 size(400,200);
 //this doesn't demonstrate best coding practice, just a simple method
 //create the MenuBar Object
 menuListen = new myMenuListener();
 myMenu = new MenuBar();

 //create the top level button
 topButton = new Menu("Colours");

 //create all the Menu Items and add the menuListener to check their state.
 item1 = new MenuItem("Red");
 item1.addActionListener(menuListen);
 item2 = new MenuItem("Green");
 item2.addActionListener(menuListen);
 item3 = new MenuItem("Blue");
 item3.addActionListener(menuListen);
 item4 = new MenuItem("Yellow");
 item4.addActionListener(menuListen);
 item5 = new MenuItem("Black");
 item5.addActionListener(menuListen);
 item5.enable(false);

 //add the items to the top level Button
 topButton.add(item1);
 topButton.add(item2);
 topButton.add(item3);
 topButton.add(item4);
 topButton.add(item5);

 //add the button to the menu
 myMenu.add(topButton);

 //add the menu to the frame!
 frame.setMenuBar(myMenu);

}

void draw(){
 // get the current menu state
 background(bg);
}

//this menuListener object is largely ripped off from http://java.sun.com/docs/books/tutorial/uiswing/examples/components/MenuDemoProject/src/components/MenuDemo.java

class myMenuListener implements ActionListener, ItemListener{

 myMenuListener(){

 }

 public void actionPerformed(ActionEvent e) {
   MenuItem source = (MenuItem)(e.getSource());
   String s = "Action event detected."
     + "    Event source: " + source.getLabel()
     + " (an instance of " + getClassName(source) + ")";
   println(s);
   
   //this part changes the background colour
   if(source.getLabel().equals("Red")){
     bg = color(220,50,0);
   }
   if(source.getLabel().equals("Blue")){
     bg = color(30,100,255);
   }
   if(source.getLabel().equals("Green")){
     bg = color(40,200,0);
   }
   if(source.getLabel().equals("Yellow")){
     bg = color(220,220,0);
   }
   if(source.getLabel().equals("Black")){
     bg = color(0);
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

