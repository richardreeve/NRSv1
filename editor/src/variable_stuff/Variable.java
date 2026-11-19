package variable_stuff;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import world_objects.*;// for node/container/link objects
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import main_method.GUI; // Main Method

import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.awt.font.TextLayout;
import java.awt.font.FontRenderContext;
import javax.swing.*;

import java.util.*;

import java.awt.*;
import java.awt.event.*;
import java.applet.Applet;
import java.awt.image.*;

import javax.swing.event.*;

/** By Anthony Mckale 2003 - this in the base class which variable descriptions  are stored in , this class as with world_object should be interface classes but didnt thinking about this till late on*/

public class Variable {
    public String Name;
    public Object Default;
    public String desc = " ";
    /** This is the main component everything is put inside */
    public  JComponent panel = new JPanel();
    
    /** This is a unique number given to each different variable to indentify one another*/
    public int serial;
    
  public LinkedList List;

    /** This is the method that adds the variabel to the editing box*/
    public void make_panel(JPanel editing, vars current ,vars defaulted){
	System.err.println("I should never be used make_panel(JPanel editing, vars current ,vars defaulted) in Variable");
    }
    
    /** This method takes a input and check if it is valid member of the variable ie a String input would be needed by variable_string */
    public boolean valid(Object input){
	System.err.println("I should never be used valid(Object input) in Variable");
	return true;
    }
    
    /** This method returns the value contained with the editor pane*/
    public Object getInput(){
	System.err.println("I should never be used getInput() in Variable");
	return null;
    }
    
    /** This method is for checking if two vars both belongs to this variable type and instance of variable type */
    boolean check_two_vars(vars a, vars b){
	return check_vars_one_of_us(a) && check_vars_one_of_us(b);
    }
    
    /** This method is for checking if a vars belongs to this variable type and instance of variable type */
    private boolean check_vars_one_of_us(vars lamb){
	Variable slaughter = this;
	return (lamb.serial == slaughter.serial );
    }
}
