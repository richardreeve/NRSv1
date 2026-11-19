/***************************************************************************************************/
/* By Anthony Mckale 2003  - this is contains all the world objects for internal representation of the neuron system */
/***************************************************************************************************/

package world_objects;

/* for package / links */
import misc.printer; // for printing (doesnt work yet)
import gui_stuff.*;//Stuff for drawing/making gui
import xml_parser.*;//Stuff for inputing/output xml files/reading xml types description file
import variable_stuff.*;//Stuff for representing variables of different types
import main_method.GUI; // Main Method

/* World Object includes */
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.awt.font.TextLayout;
import java.awt.font.FontRenderContext;
import javax.swing.*;

import java.awt.*;
import java.awt.event.*;
import java.applet.Applet;
import java.awt.image.*;

import java.util.*; // For Linked Lists

/***************/
/* NEURON       */
/***************/

public class Dummy_Neuron extends World_Object {
    static Color normal_box_wall = Color.blue;
    static Color normal_box_inside = Color.pink;

    static Color normal_text_network = Color.black;
    static Color normal_text_name = Color.black;
    static Color normal_text_type = Color.black;

    static Color selected_box_wall = Color.blue;
    static Color selected_box_inside = Color.red;
    
    static Color selected_text_network = Color.black;
    static Color selected_text_name = Color.black;
    static Color selected_text_type = Color.black;
    
    // Used to give an orginal name
     private static int number = 1; // The keeps track of how many neurons have been made
    
    // Objects used for drawing
    private Rectangle rect;
    
    /* Next two constructors initize the Neuron and set default Neuron values */
    
    public Dummy_Neuron() {
	set_up_vars(false);
parent = GUI.world; 
    }
    /*
    public Dummy_Neuron(int newx, int newy) {
	x = newx; y = newy;
	set_up_vars();
    }
    
    public Dummy_Neuron(int newx, int newy, World_Object parent) {
	x = newx; y = newy;
	set_up_vars();
	this.parent = parent;
    }
    */
    public Dummy_Neuron(int Type) {
	Connected = false;
	this.Type = Type;
	set_up_vars(true);
parent = GUI.world; 
    }

    /** This is a Method to set up the Neurons
     * rather than type the same thing twice */

    private void set_up_vars(boolean increase){
	// Set Up name
	Object_Type= Neuron;

	this.id = id_ref++;
	if(increase){
	    name = "Neuron_" + Integer.toString(number++);
	}else{
	    name="XXX";
	}
		
	// Set Up Variables Stuff
	list_of_Parts = new LinkedList();
	
	Type_Names = arrays.Neuron_Names;
	Parameters_used_by_Type = arrays.Neuron_Vars;

	// Set up own parameters
	setup_vars();

    }
    
    void drawme(Graphics2D g2, int draw_mode){
	if(Connected){
	    if(draw_mode == Selected_Draw_Mode ||(draw_mode == Normal_Draw_Mode && !(GUI.world.Currently_Selected_List.contains((Object)this))) ){
		Color box_wall = normal_box_wall;
		Color box_inside = normal_box_inside;
		Color text_network = normal_text_network;
		Color text_name = normal_text_name;
		Color text_type = normal_text_type;

		if(draw_mode == Selected_Draw_Mode ){
		    box_wall =  selected_box_wall;
		    box_inside = selected_box_inside;
		    text_network = selected_text_network;
		    text_name = selected_text_name;
		    text_type = selected_text_type;
		}
	
		
		//draw Neuron
		
		//Draw Big Box
		g2.setColor(box_wall);
		rect = new Rectangle(x, y, 50, 50);
		g2.draw(rect);
		g2.fill(rect);
		
		//Draw Smaller Box
		g2.setColor(box_inside);
		rect = new Rectangle(x+2, y+2, 46, 46);
		g2.draw(rect);
		g2.fill(rect);
		
		//Write Pretty Name
		GUI.Antialias_on(g2);
		
		// Write Name
		FontRenderContext frc = g2.getFontRenderContext();
		Font font = g2.getFont();
		TextLayout tl = new TextLayout(name, font, frc);
		float sw = (float) tl.getBounds().getWidth();
		float sh = (float) tl.getAscent() + tl.getDescent();
		g2.setColor(text_name);
		tl.draw(g2, (float) x+25-sw/2, y+25+sh/2);
		
		// Write Object Belongs to below
		g2.setColor(text_network);
		TextLayout parent_text = new TextLayout(parent.name, font, frc);
		sw = (float) parent_text.getBounds().getWidth();
		sh = (float) parent_text.getAscent() + parent_text.getDescent();
		parent_text.draw(g2, (float) x+25-sw/2, (float)(y+25+1.5*sh));
		
		// Write Type Belongs to below
		g2.setColor(text_type);
		TextLayout Type_Name = new TextLayout(Type_Names[Type], font, frc);
		sw = (float) Type_Name.getBounds().getWidth();
		sh = (float) Type_Name.getAscent() + Type_Name.getDescent();
		Type_Name.draw(g2, (float) x+25-sw/2, (float)(y+25-0.5*sh));
		
		GUI.Antialias_off(g2);
	    }
	}
    }
    
    public String getType(){
	return Type_Names[Type];
    }    
    
    /* Method check wiether or not a point X, Y is within the Object */

    World_Object  hitme(int X, int Y) {
	if(rect !=null)
	    if(rect.contains(X, Y)) return this;
	return null;
    }

    public vars get_inheritance(int serial){
	return parent.get_inheritance(serial);
    }
    
    /** This returns a list of the synapse connected to the neuron to and from 
     * and this neuron at the end, used for when moving neuron and drawing the result
     * only component connected to the neuron need edited a*/
    public LinkedList get_list_of_connected_stuff(){
	LinkedList stuff = new LinkedList();
	if(GUI.world.Synapse_List != null)
	    for(int counter =0; counter < GUI.world.Synapse_List.size() ;counter++) {
		Link current_synapse = (Link)GUI.world.Synapse_List.get(counter);
		if(current_synapse.To == this || current_synapse.From == this)
		    stuff.add((Object)current_synapse);
	    }
	stuff.add((Object)this);
	return stuff;
    }
}
